#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tsalgo/random.h>
#include <tsalgo/bufsort.h>

#include <progress.h>

#define ELEMENTS 4096
#define KEYSIZE  sizeof(ts_algo_key_t)
#define BUFSIZE ELEMENTS*KEYSIZE

ts_algo_key_t buf[ELEMENTS];

ts_algo_cmp_t mycompare(ts_algo_key_t *left,
                        ts_algo_key_t *right)
{
	if (*left < *right) return ts_algo_cmp_less;
	if (*left > *right) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

void init_keys() {
	int i;
	for(i=0;i<ELEMENTS;i++) {
		buf[i]=randomUnsigned(1,1000000);
	}
}

ts_algo_bool_t testquick() {
	ts_algo_key_t *sorted, *sorted2;
	int i;
	
	sorted=ts_algo_sort_buf_quick((const ts_algo_sort_buf_t)buf,
	                              BUFSIZE,KEYSIZE,
	                              (ts_algo_compare_t)&mycompare);
	if (sorted==NULL) return FALSE;
	for(i=0;i<ELEMENTS-1;i++) {
		if (sorted[i] > sorted[i+1]) return FALSE;
	}
	sorted2=ts_algo_sort_buf_quick((const ts_algo_sort_buf_t)sorted,
	                              BUFSIZE,KEYSIZE,
	                              (ts_algo_compare_t)&mycompare);
	if (sorted != sorted2) return FALSE;

	ts_algo_sort_buf_release((ts_algo_sort_buf_t)buf,
	                         (ts_algo_sort_buf_t)sorted);
	return TRUE;
}

ts_algo_bool_t testmerge() {
	ts_algo_key_t *sorted, *sorted2;
	int i;
	
	sorted=ts_algo_sort_buf_merge((const ts_algo_sort_buf_t)buf,
	                              BUFSIZE,KEYSIZE,
	                              (ts_algo_compare_t)&mycompare);
	if (sorted==NULL) return FALSE;
	for(i=0;i<ELEMENTS-1;i++) {
		if (sorted[i] > sorted[i+1]) return FALSE;
	}
	sorted2=ts_algo_sort_buf_merge((const ts_algo_sort_buf_t)sorted,
	                               BUFSIZE,KEYSIZE,
	                               (ts_algo_compare_t)&mycompare);
	if (sorted != sorted2) return FALSE;

	ts_algo_sort_buf_release((ts_algo_sort_buf_t)buf,
	                         (ts_algo_sort_buf_t)sorted);
	return TRUE;
}

int main() {
	progress_t p;
	int i, it=1000;

	init_rand();
	init_keys();

	init_progress(&p,stdout,it);
	for(i=0;i<it;i++) {
		if (!testquick()) {
			printf("\nquicksort failed\n");
			return EXIT_FAILURE;
		}
		if (!testmerge()) {
			printf("\nmergesort failed\n");
			return EXIT_FAILURE;
		}
		update_progress(&p,i);
	}
	close_progress(&p); printf("\n");
	printf("all tests passed!\n");
	return EXIT_SUCCESS;
}
