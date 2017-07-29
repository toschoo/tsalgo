#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tsalgo/random.h>
#include <tsalgo/filesort.h>

#define BLOCK    8192
#define KEYSIZE  sizeof(ts_algo_key_t)
#define BUFSIZE  65536
#define FILESIZE TS_ALGO_MEGA
#define PATH  "rsc/unsorted1.bin"
#define PATH2 "rsc/unsorted2.bin"
#define OUTPATH "rsc/sorted.bin"

ts_algo_cmp_t mycompare(ts_algo_key_t *left,
                        ts_algo_key_t *right)
{
	if (*left < *right) return ts_algo_cmp_less;
	if (*left > *right) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

char createFile(char *fname) {
	int    i,j;
	char   tmp[BLOCK];
	size_t nb = FILESIZE/BLOCK;
	ts_algo_key_t k;
	size_t s = sizeof(ts_algo_key_t);
	FILE *stream;

	stream = fopen(fname,"w");
	if (stream == NULL) return -1;

	for(j=0;j<nb;j++) {
		for(i=0;i<BLOCK;i+=s) {
			k=randomUnsigned(1,1000000);
			memcpy(tmp+i,&k,s);
		}
		if (fwrite(tmp,BLOCK,1,stream) != 1) {
			fclose(stream); return -2;
		}
	}
	fclose(stream);
	return 0;
}

ts_algo_bool_t validate(char *fname,size_t size) {
	FILE *stream;
	char  buf[BLOCK];
	int   i,j;
	ts_algo_key_t k1,k2;

	stream = fopen(fname,"r");
	if (stream == NULL) return FALSE;

	j=BLOCK;
	k1=0;k2=0;
	for(i=0;i<size;i+=KEYSIZE) {
		if (j==BLOCK) {
			if (fread(buf,BLOCK,1,stream) != 1) {
				printf("cannot read\n");
				fclose(stream); return FALSE;
			}
			j=0;
		}
		if ((i/KEYSIZE)%2!=0) k1=k2;
		memcpy(&k2,buf+j,KEYSIZE); j+=KEYSIZE;
		/*
		printf("comparing %llu and %llu\n",
		      (unsigned long long)k1,
		      (unsigned long long)k2);
		*/
		if (k1 > k2) {
			printf("error at %lu: %llu > %llu\n", 
			      (unsigned long)i,
			      (unsigned long long)k1,
			      (unsigned long long)k2);
			fclose(stream); return FALSE;
		}
	}
	fclose(stream);
	return TRUE;
}

ts_algo_bool_t testSort(int f) {
	ts_algo_rc_t rc;
	
	rc = ts_algo_sort_file(PATH,OUTPATH,f*BUFSIZE,FILESIZE,KEYSIZE,
	                       (ts_algo_compare_t)&mycompare);
	if (rc != TS_ALGO_OK) {
		printf("file sorting terminated with: %d\n",rc);
		return FALSE;
	}
	return TRUE;
	return validate(OUTPATH,FILESIZE);
}

int main() {
	char rc;

	init_rand();

	printf("testing with buffer size %lu\n",(unsigned long)BUFSIZE);
	remove(PATH);
	remove(PATH2);
	remove(OUTPATH);
	rc = createFile(PATH);
	if (rc != 0) {
		printf("cannot create file %s\n", PATH);
		return EXIT_FAILURE;
	}
	if (!testSort(1)) {
		printf("sort failed\n");
		return EXIT_FAILURE;
	}
	printf("ok\n");
	return EXIT_SUCCESS;
}
