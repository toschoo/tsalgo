/* ========================================================================
 * (c) Tobias Schoofs, 2016
 * ========================================================================
 * FileSort
 * ========================================================================
 * Provides functions for external sorting.
 * It is based on the tape sorting alorithms presented
 * in Knuth TAOCP, Volume 3. 
 * ========================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <tsalgo/bufsort.h> 
#include <tsalgo/filesort.h> 

#define BLOCK 8192

/* ------------------------------------------------------------------------
 * Read file 
 * with optimal buffer size and fill target buffer on the way
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t fillbuf(FILE  *stream, 
                            char  *buf,
                            size_t bufsize)
{
	size_t j,s,n;

	s=bufsize;j=0;
	while(s>0) {
		if (s>=BLOCK) {
			s-=BLOCK;n=BLOCK;
		} else {
			n=s;s=0;
		}
		if (fread(buf+j,n,1,stream) != 1) return TS_ALGO_FREAD;
		j+=n;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Write file 
 * with optimal buffer size reading data from source buffer 
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t writebuf(FILE  *ostream,
                             char  *buf,
                             size_t bufsize)
{
	size_t j,s,n;

	s=bufsize;j=0;
	while(s>0) {
		if (s>=BLOCK) {
			n=BLOCK;s-=BLOCK;
		} else {
			n=s;s=0;
		}
		if (fwrite(buf+j,n,1,ostream) != 1) return TS_ALGO_FWRITE;
		j+=n;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Create empty file
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t creatEmpty(char *fname,size_t filesize) {
	FILE *stream;
	char tmp[BLOCK];
	size_t s,n;
	
	memset(tmp,0,BLOCK);

	/* 'b' means binary and is
	 * included to ease portability */
	stream = fopen(fname,"wb"); 
	if (stream == NULL) return TS_ALGO_FOPEN;

	s=filesize;
	while(s>0) {
		if (s>BLOCK) {
			n=BLOCK;s-=BLOCK;
		} else {
			n=s;s=0;
		}
		if (fwrite(tmp,n,1,stream) != 1) {
			fclose(stream);
			return TS_ALGO_FWRITE;
		}
	}
	fclose(stream);
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Sort file buffer-wise
 * using one of the buffer sort algorithms (qicksort)
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t sortBuffers(char *fname,
                                char *oname,
                                size_t bufsize,
                                size_t filesize,
                                size_t storesize,
                                ts_algo_compare_t compare)
{
	FILE  *stream=NULL,*ostream=NULL;
	char  *buf=NULL,*buf2=NULL; 
	ts_algo_rc_t rc = TS_ALGO_OK;
	size_t restsize,s;

	/* open file */
	stream = fopen(fname,"rb");
	if (stream == NULL) {
		rc = TS_ALGO_FOPEN;
		goto cleanup;
	}
	/* open out file */
	ostream = fopen(oname,"wb");
	if (ostream == NULL) {
		rc = TS_ALGO_FOPEN;
		goto cleanup;
	}
	/* allocate buffer */
	buf = malloc(bufsize);
	if (buf == NULL) {
		rc = TS_ALGO_NO_MEM;
		goto cleanup;
	}
	/* sort buffer by buffer */
	restsize = filesize;
	while(restsize>0) {
		if (restsize>bufsize) s=bufsize;
		else s=restsize;
		rc = fillbuf(stream,buf,s);
		if (rc != TS_ALGO_OK) goto cleanup;
		buf2 = ts_algo_sort_buf_quick(buf,s,storesize,compare);
		if (buf2 == NULL) {
			rc = TS_ALGO_NO_MEM;
			goto cleanup;
		}
		rc = writebuf(ostream,buf2,s);
		ts_algo_sort_buf_release(buf,buf2);
		if (rc != TS_ALGO_OK) goto cleanup;
		restsize-=s;
	}
cleanup:
	if (stream != NULL) fclose(stream);
	if (ostream != NULL) fclose(ostream);
	if (buf  != NULL) free(buf);
	return rc;
}

/* ------------------------------------------------------------------------
 * Merge two sorted buffers
 * with bufsize = n*basesize
 * we use memory buffers of basesize
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t merge(FILE             *src1, 
                          FILE             *src2,
                          FILE             *trg,
                          size_t            basesize,
                          size_t            bufsize1,
                          size_t            bufsize2,
                          size_t            storesize,
                          ts_algo_compare_t compare)
{
	size_t i1,i2,j1,j2,z;
	size_t bs1,bs2,s;
	char *buf1;
	char *buf2;
	char *buf3;
	ts_algo_cmp_t cmp;
	ts_algo_rc_t  rc=TS_ALGO_OK;

	bs1=basesize<bufsize1?basesize:bufsize1;
	bs2=basesize<bufsize2?basesize:bufsize2;

	buf1 = malloc(bs1); 
	if (buf1 == NULL) return TS_ALGO_NO_MEM;
	if (bs2 > 0) {
		buf2 = malloc(bs2); 
		if (buf2 == NULL) {
			free(buf1);
			return TS_ALGO_NO_MEM;
		}
	} else buf2 = NULL;

	buf3 = malloc(basesize); 
	if (buf3 == NULL) {
		free(buf1); 
		if (buf2 != NULL) free(buf2);
		return TS_ALGO_NO_MEM;
	}
	i1=bs1;j1=0;j2=0;z=0;
	i2=bufsize2==0?0:bs2;
	while(j1<bufsize1||j2<bufsize2) {
		if (i1==bs1 && j1<bufsize1) {
			s=bufsize1-j1;s=s>bs1?bs1:s;
			rc = fillbuf(src1,buf1,s);
			if (rc != TS_ALGO_OK) {
				free(buf1);free(buf3);
				if (buf2 != NULL) free(buf2);
				return rc;
			}
			i1=0;
		}
		if (i2==bs2 && j2<bufsize2) {
			s=bufsize2-j2;s=s>bs2?bs2:s;
			rc = fillbuf(src2,buf2,s);
			if (rc != TS_ALGO_OK) {
				free(buf1);free(buf3);
				if (buf2 != NULL) free(buf2);
				return rc;
			}
			i2=0;
		}
		if (j1==bufsize1) cmp = ts_algo_cmp_greater;
		else if (j2==bufsize2) cmp = ts_algo_cmp_less;
		else cmp = compare(buf1+i1,buf2+i2);
		if (cmp == ts_algo_cmp_less || cmp == ts_algo_cmp_equal)
		{
			memcpy(buf3+z,buf1+i1,storesize);
			i1+=storesize;j1+=storesize;
		} else {
			memcpy(buf3+z,buf2+i2,storesize);
			i2+=storesize;j2+=storesize;
		}
		z+=storesize;
		if (z==basesize) {
			rc = writebuf(trg,buf3,basesize);
			if (rc != TS_ALGO_OK) {
				free(buf1);free(buf3);
				if (buf2 != NULL) free(buf2);
				return rc;
			}
			z=0;
		}
	}
	if (z>0) rc = writebuf(trg,buf3,z);
	free(buf1);free(buf3);
	if (buf2 != NULL) free(buf2);
	return rc;
}

/* ------------------------------------------------------------------------
 * Merge n sorted buffers pairwise and repeat
 * until the size of one buffer equals or exceeds filesize,
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t mergeBuffers(FILE             *hlp11,
                                 FILE             *hlp12,
                                 FILE             *hlp21,
                                 FILE             *hlp22,
                                 size_t            bufsize,
                                 size_t            filesize,
                                 size_t            storesize,
                                 size_t           *step,
                                 ts_algo_compare_t compare)
{
	size_t x   = pow(2,*step);
	size_t i,n = x*bufsize;
	size_t bs1=0,bs2=0;
	size_t rest;
	ts_algo_rc_t rc = TS_ALGO_OK;

	/* everything is merged */
	if (n >= filesize) return TS_ALGO_OK;

	/* set filedescriptors to point to beginning of the files */
	if (fseek(hlp11,0,SEEK_SET) != 0) return TS_ALGO_FSEEK;
	if (fseek(hlp12,0,SEEK_SET) != 0) return TS_ALGO_FSEEK;
	if (fseek(hlp21,0,SEEK_SET) != 0) return TS_ALGO_FSEEK;

	for (i=0;i<filesize;i+=bs1+bs2) {
		/* if the filesize is not a multiple of 2,
		 * there will at some point be a remainder
		 * that must be explicitly taken care of */
		rest = filesize - i;
		if (rest < 2*n) { /* there is a remainder */
			if (rest < n) { /* we have a null-round */
				bs1=rest;bs2=0;
			} else { /* we merge with a smaller rest */
				bs1=n;bs2=rest-n;
			}
		} else { /* everything fits */
			bs1=n;bs2=n;
		}
		/* move the second helper file forward 
		 * by the size of the first buffer */
		if (fseek(hlp12,bs1,SEEK_CUR) != 0) return TS_ALGO_FSEEK;
		/* debug
		long h1=ftell(hlp11);
		long h2=ftell(hlp12);
		*/
		rc = merge(hlp11,hlp12,hlp21,
		           bufsize,bs1,bs2,
		           storesize,compare);
		if (rc != TS_ALGO_OK) return rc;
		/* debug
		long h3=ftell(hlp11);
		long h4=ftell(hlp12);
		*/
		/* move the first helper file forward 
		 * by the size of the second buffer */
		if (fseek(hlp11,bs2,SEEK_CUR) != 0) return TS_ALGO_FSEEK;
		/* debug
		printf("step %lu: merging %09lu-%09lu (%lu) and %09lu-%09lu (%lu)\n",
		       *step,h1,h3,bs1,h2,h4,bs2);
		*/
	}
	/* flush the outgoing buffer */
	if (fflush(hlp21) != 0) return TS_ALGO_FFLUSH;
	/* next recursion step */
	(*step)++;
	return mergeBuffers(hlp21,hlp22,
	                    hlp11,hlp12,
	                    bufsize,
	                    filesize,
	                    storesize,
	                    step,compare);
}

/* ------------------------------------------------------------------------
 * External sorting
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_sort_file(char             *fname,
                               char             *oname,
                               size_t            bufsize,
                               size_t            filesize,
                               size_t            storesize,
                               ts_algo_compare_t compare)
{
	ts_algo_rc_t rc = TS_ALGO_OK;
	size_t s;
	size_t step=0;
	char  *oname1 = NULL,*oname2 = NULL;
	char  *o,*r;
	FILE  *hlp11,*hlp12,*hlp21,*hlp22;

	if (bufsize <  storesize)    return TS_ALGO_INVALID;
	if (bufsize%storesize != 0)  return TS_ALGO_INVALID;
	if (bufsize < BLOCK)         return TS_ALGO_INVALID;
	if (filesize <  bufsize)     return TS_ALGO_INVALID;
	if (filesize%storesize != 0) return TS_ALGO_INVALID;

	/* sort buffers */
	s=strnlen(fname,MAXPATH);
	if (s>=MAXPATH) return TS_ALGO_INVALID;
	oname1 = malloc(s+1+19);
	if (oname1 == NULL) return TS_ALGO_NO_MEM;
	sprintf(oname1,"%s.tmp.ts.algo.sort.1",fname);
	rc = sortBuffers(fname,oname1,bufsize,filesize,storesize,compare);
	if (rc != TS_ALGO_OK) {
		free(oname1);
		return rc;
	}
	/* create helper file */
	oname2 = malloc(s+1+19);
	if (oname2 == NULL) {
		free(oname2);
		return TS_ALGO_NO_MEM;
	}
	sprintf(oname2,"%s.tmp.ts.algo.sort.2",fname);
	rc = creatEmpty(oname2,filesize);
	if (rc != TS_ALGO_OK) {
		free(oname1);free(oname2);
		return rc;
	}
	/* helper file descriptors */
	hlp11 = fopen(oname1,"r+b");
	if (hlp11 == NULL) {
		free(oname1);free(oname2);
		return TS_ALGO_FOPEN;
	}
	hlp12 = fopen(oname1,"r+b");
	if (hlp12 == NULL) {
		fclose(hlp11);
		free(oname1);free(oname2);
		return TS_ALGO_FOPEN;
	}
	hlp21 = fopen(oname2,"r+b");
	if (hlp21 == NULL) {
		fclose(hlp11);fclose(hlp12);
		free(oname1);free(oname2);
		return TS_ALGO_FOPEN;
	}
	hlp22 = fopen(oname2,"r+b");
	if (hlp22 == NULL) {
		fclose(hlp11);fclose(hlp12);fclose(hlp21);
		free(oname1);free(oname2);
		return TS_ALGO_FOPEN;
	}
	/* merge buffers */
	rc = mergeBuffers(hlp11,hlp12,
	                  hlp21,hlp22,
	                  bufsize,
	                  filesize,
	                  storesize,
	                  &step,compare);
	fclose(hlp11);fclose(hlp12);
	fclose(hlp21);fclose(hlp22);
	if (rc != TS_ALGO_OK) {
		free(oname1);free(oname2);
		return rc;
	}
	/* return the file holding the final result,
	   remove the other one */
	if (step%2==0) {
		o=oname1;r=oname2;
	} else {
		o=oname2;r=oname1;
	}
	if (rename(o,oname) != 0) rc = TS_ALGO_RENAME;
	remove(r);free(oname1);free(oname2);
	return rc;
}

/* ------------------------------------------------------------------------
 * Merge two files
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_merge_files(char             *f1name,
                                 char             *f2name,
                                 char             *oname,
                                 size_t            filesize1,
                                 size_t            filesize2,
                                 size_t            storesize,
                                 ts_algo_compare_t compare)
{
	FILE *src1,*src2,*trg;
	ts_algo_rc_t rc = TS_ALGO_OK;

	src1 = fopen(f1name,"rb");
	if (src1 == NULL) return TS_ALGO_FOPEN;
	src2 = fopen(f2name,"rb");
	if (src2 == NULL) {
		fclose(src1);
		return TS_ALGO_FOPEN;
	}
	trg = fopen(oname,"wb");
	if (trg == NULL) {
		fclose(src1);fclose(src2);
		return TS_ALGO_FOPEN;
	}
	rc = merge(src1,src2,trg,
	           8192,filesize1,filesize2,
	           storesize,compare);
	fclose(src1);fclose(src2),fclose(trg);
	if (rc != TS_ALGO_OK) remove(oname);
	return rc;
}
