#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tsalgo/random.h>
#include <tsalgo/filesort.h>

#define BLOCK    8192
#define ELEMENTS 2048
#define KEYSIZE  sizeof(ts_algo_key_t)
#define BUFSIZE ELEMENTS*KEYSIZE
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

char createFile(char *fname,size_t fsize) {
	int    i,j,r;
	char   tmp[BLOCK];
	size_t nb = fsize/BLOCK;
	ts_algo_key_t k;
	size_t s = sizeof(ts_algo_key_t);
	size_t lm,m = BLOCK;
	FILE *stream;

	stream = fopen(fname,"w");
	if (stream == NULL) return -1;

	if (fsize%BLOCK != 0) {
		lm = fsize-nb*BLOCK;nb++;
		if (lm < s) {
			printf("%lu is not a multiple of %llu\n",
			        (unsigned long)BLOCK, 
			        (unsigned long long)s);
			return -1;
		}
	} else lm=m;

	for(j=nb;j>0;j--) {
		if (j==1) m=lm;
		for(i=0;i<m;i+=s) {
			k=randomUnsigned(1,500000);
			memcpy(tmp+i,&k,s);
		}
		if ((r=fwrite(tmp,m,1,stream)) != 1) {
			printf("couldn't write file: %d\n",r);
			fclose(stream); return -2;
		}
	}
	fclose(stream);
	return 0;
}

void showFile(char *fname, size_t size) {
	char           buf[BLOCK];
	FILE          *stream;
	ts_algo_key_t *keys;
	size_t         max,s,i,j;

	stream = fopen(fname,"r");
	if (stream == NULL) {
		printf("cannot open\n");
		return;
	}
	i=size;
	while(i>0) {
		if (i>BLOCK) {
			s=BLOCK;i-=BLOCK;
		} else {
			s=i;i=0;
		}
		if (fread(buf,s,1,stream) != 1) {
			printf("cannot read\n");
			fclose(stream);return;
		}
		max=s/sizeof(ts_algo_key_t);
		keys=(ts_algo_key_t*)buf;
		for(j=0;j<max;j++) {
			printf("%llu\n",(unsigned long long)keys[j]);
		}
	}
	fclose(stream);
}

ts_algo_bool_t validate(char *fname,size_t size) {
	FILE       *stream;
	char        buf[BLOCK];
	int         i,j=BLOCK;
	size_t      m=BLOCK;
	ts_algo_key_t k1,k2;

	stream = fopen(fname,"r");
	if (stream == NULL) return FALSE;

	k1=0;k2=0;
	for(i=0;i<size;i+=KEYSIZE) {
		if (j==BLOCK) {
			if (size-i<BLOCK) m = size-i;
			if (fread(buf,m,1,stream) != 1) {
				printf("cannot read\n");
				fclose(stream); return FALSE;
			}
			j=0;
		}
		if ((i/KEYSIZE)%2==0) k1=k2;
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

typedef struct {
	ts_algo_key_t key;
	int      expected;
	int         found;
} val_t;

val_t *reg;

ts_algo_bool_t initRegister(size_t size) {
	size_t i;

	reg = malloc(size*sizeof(val_t));
	if (reg == NULL) {
		printf("not enough memory\n");
		return FALSE;
	}
	for(i=0;i<size;i++) {
		reg[i].key      = 0;
		reg[i].expected = 0;
		reg[i].found    = 0;
	}
	return TRUE;
}

ts_algo_bool_t fillRegister(size_t size, char *fname) {
	char   buf[BLOCK];
	ts_algo_key_t *keys;
	FILE  *stream;
	size_t z,j,i,s;
	size_t max;
	int    rc;

	stream = fopen(fname,"r");
	if (stream == NULL) {
		printf("cannot open file\n");
		return FALSE;
	}
	/*
	printf("processing %zu blocks: ",size/BLOCK);
	*/
	printf("0000"); fflush(stdout);
	// for (i=0;i<size;i+=BLOCK) {
	i=size;
	while(i>0) {
		if (i>BLOCK) {
			s=BLOCK;i-=BLOCK;
		} else {
			s=i;i=0;
		}
		rc=fread(buf,s,1,stream);
		if (rc != 1) {
			printf("cannot read file: %d\n",rc);
			fclose(stream);return FALSE;
		}
		keys = (ts_algo_key_t*)buf;
		printf("\b\b\b\b");printf("%04zu",i/BLOCK);fflush(stdout);
		max=s/sizeof(ts_algo_key_t);
		for(j=0;j<max;j++) {
			for (z=0;z<size;z++) {
				if (reg[z].key == keys[j]) {
					reg[z].expected++; break;
				}
				if (reg[z].expected == 0) {
					reg[z].key = keys[j];
					reg[z].expected=1; break;
				}
			}
		}
	}
	// printf("\n");
	fclose(stream); return TRUE;
}

ts_algo_bool_t checkRegister(size_t size, char *fname) {
	char buf[BLOCK];
	ts_algo_key_t *keys;
	FILE  *stream;
	size_t max;
	size_t z,j,i,s;

	stream = fopen(fname,"r");
	if (stream == NULL) {
		printf("cannot open file\n");
		return FALSE;
	}
	/*
	printf("processing %zu blocks: ",size/BLOCK);
	*/
	printf("0000");fflush(stdout);
	i=size;
	while(i>0) {
		if (i>BLOCK) {
			s=BLOCK;i-=BLOCK;
		} else {
			s=i;i=0;
		}
		if (fread(buf,s,1,stream) != 1) {
			printf("cannot read file\n");
			fclose(stream);return FALSE;
		}
		keys=(ts_algo_key_t*)buf;
		printf("\b\b\b\b");printf("%04zu",i/BLOCK);fflush(stdout);
		max=s/sizeof(ts_algo_key_t);
		for(j=0;j<max;j++) {
			for (z=0;z<size;z++) {
				if (reg[z].key == keys[j]) {
					reg[z].found++; break;
				}
				if (reg[z].expected == 0) {
					printf("unexpected key: %llu\n",
					      (unsigned long long)keys[j]);
					fclose(stream);return FALSE;
				}
			}
		}
	}
	// printf("\n");
	fclose(stream); return TRUE;
}

ts_algo_bool_t validateRegister(size_t size) {
	size_t i;
	char rc = TRUE;
	int c=0,sum=0;

	for(i=0;i<size;i++) {
		if (reg[i].expected != reg[i].found) {
			/*
			printf("register is inconsistent on ");
			printf("%zu (%llu): %d - %d\n",i,
			       (unsigned long long)reg[i].key,
			       reg[i].expected,reg[i].found);
			return FALSE;
			*/
			sum+=reg[i].expected-reg[i].found;
			c++;
			rc=FALSE;
		}
	}
	if (c>0)
		printf("%d inconsistencies summing up to %d\n",c,sum);
	return rc;
}

ts_algo_bool_t testSortHeavy(int f, size_t fsize) {
	ts_algo_rc_t rc;

	if (!initRegister(fsize)) {
		printf("cannot init register\n");
		return FALSE;
	}
	if (!fillRegister(fsize,PATH)) {
		printf("cannot fill register\n");
		return FALSE;
	}
	// printf("registered\n");
	rc = ts_algo_sort_file(PATH,OUTPATH,f*BUFSIZE,fsize,KEYSIZE,
	                       (ts_algo_compare_t)&mycompare);
	if (rc != TS_ALGO_OK) {
		printf("file sorting terminated with: %d\n",rc);
		return FALSE;
	}
	// printf("sorted\n");
	if (!checkRegister(fsize,OUTPATH)) { 
		printf("cannot check register\n");
		return FALSE;
	}
	// printf("Register checked\n");
	if (!validateRegister(fsize)) {
		printf("cannot validate register\n");
		return FALSE;
	}
	printf("\b\b\b\b\b\b\b\b");fflush(stdout);
	// printf("Register validate\n");
	free(reg);reg=NULL;
	return validate(OUTPATH,fsize);
}

ts_algo_bool_t testSort(int f, size_t fsize) {
	ts_algo_rc_t rc;

	rc = ts_algo_sort_file(PATH,OUTPATH,f*BUFSIZE,fsize,KEYSIZE,
	                       (ts_algo_compare_t)&mycompare);
	if (rc != TS_ALGO_OK) {
		printf("file sorting terminated with: %d\n",rc);
		return FALSE;
	}
	return validate(OUTPATH,fsize);
}

ts_algo_bool_t testMerge(size_t fs1, size_t fs2) {
	ts_algo_rc_t rc;

	rc = ts_algo_merge_files(PATH,PATH2,OUTPATH,
	                         fs2,fs1,KEYSIZE,
	                        (ts_algo_compare_t)&mycompare);
	if (rc!=TS_ALGO_OK) {
		printf("merging terminated with: %d\n",rc);
		return FALSE;
	}
	return validate(OUTPATH,fs1+fs2);
}

size_t randomFileSize(size_t basesize, size_t keysize) {
	return basesize + (keysize * (rand()%10000));
}

size_t randomBufSize() {
	return pow(2,rand()%4);
}

int main() {
	int    i;
	char   rc;
	size_t fs,fs2,bs;

	init_rand();

	printf("visual test\n");
	remove(PATH);
	remove(PATH2);
	remove(OUTPATH);
	fs=2*BLOCK;bs=1;
	rc = createFile(PATH,fs);
	if (rc != 0) {
		printf("cannot create file %s\n", PATH);
		return EXIT_FAILURE;
	}
	if (!testSort(bs,fs)) {
		printf("FAILED\n");
		return EXIT_FAILURE;
	}
	printf("\n");
	showFile(OUTPATH,fs);
	printf("\n");
	fprintf(stdout,"Heavy test...\n");
	fprintf(stdout,"000"); fflush(stdout);
	for(i=0;i<10;i++) {
		remove(PATH);
		remove(PATH2);
		remove(OUTPATH);
		fs = randomFileSize(16*BLOCK,sizeof(ts_algo_key_t));
		bs = randomBufSize();
		rc = createFile(PATH,fs);
		if (rc != 0) {
			printf("cannot create file %s\n", PATH);
			return EXIT_FAILURE;
		}
		fprintf(stdout,"\b\b\b%03d",i+1); fflush(stdout);
		if (!testSortHeavy(bs,fs)) {
			printf("FAILED\n");
			return EXIT_FAILURE;
		}
	}
	printf("\n");
	fprintf(stdout,"light tests...\n");

	fprintf(stdout,"0000"); fflush(stdout);
	for(i=0;i<25;i++) {
		remove(PATH);
		remove(PATH2);
		remove(OUTPATH);
		fs = randomFileSize(FILESIZE,sizeof(ts_algo_key_t));
		bs = randomBufSize();
		rc = createFile(PATH,fs);
		if (rc != 0) {
			printf("cannot create file %s\n", PATH);
			return EXIT_FAILURE;
		}
		if (!testSort(bs,fs)) {
			printf("sort failed\n");
			return EXIT_FAILURE;
		}
		fprintf(stdout,"\b\b\b\b%04d",i+1); fflush(stdout);
	}
	printf("\n");
	fprintf(stdout,"0000"); fflush(stdout);
	for(i=0;i<10;i++) {
		remove(PATH);
		remove(PATH2);
		remove(OUTPATH);
		fs  = randomFileSize(FILESIZE,sizeof(ts_algo_key_t));
		fs2 = randomFileSize(FILESIZE,sizeof(ts_algo_key_t));
		rc = createFile(PATH,fs);
		if (rc != 0) {
			printf("cannot create file %s\n", PATH);
			return EXIT_FAILURE;
		}
		if (!testSort(1,fs)) {
			printf("sort failed!\n");
			return EXIT_FAILURE;
		}
		rename(OUTPATH,PATH2);
		remove(PATH);
		rc = createFile(PATH,fs2);
		if (rc != 0) {
			printf("cannot create file %s\n", PATH);
			return EXIT_FAILURE;
		}
		if (!testSort(1,fs2)) {
			printf("sort failed!\n");
			return EXIT_FAILURE;
		}
		rename(OUTPATH,PATH); 
		remove(OUTPATH);
		if (!testMerge(fs,fs2)) {
			printf("merge failed\n");
			return EXIT_FAILURE;
		}
		fprintf(stdout,"\b\b\b\b%04d",i+1); fflush(stdout);
	}
	printf("\n");
	printf("all tests passed!\n");
	return EXIT_SUCCESS;
}
