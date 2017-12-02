#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <tsalgo/tree.h>
#include <tsalgo/random.h>
#include <progress.h>

typedef struct timespec timestamp_t;
int timestamp(timestamp_t *tmstp) {
	return clock_gettime(CLOCK_MONOTONIC, tmstp);
}

#define NPERSEC 1000000000

uint64_t timediff(timestamp_t *t1, timestamp_t *t2) {
	timestamp_t t3; /* t3 = t1 - t2 */

	if (t2->tv_nsec > t1->tv_nsec) {
		t3.tv_nsec = 
			(NPERSEC + t1->tv_nsec) - t2->tv_nsec;
		t3.tv_sec = (t1->tv_sec - 1) - t2->tv_sec;
	} else {
		t3.tv_nsec = t1->tv_nsec - t2->tv_nsec;
		t3.tv_sec  = t1->tv_sec  - t2->tv_sec;
	}
	return (t3.tv_sec * NPERSEC + t3.tv_nsec);
}

#define ELEMENTS 100000

ts_algo_key_t  keys[ELEMENTS];

typedef struct {
	ts_algo_key_t k1;
	ts_algo_key_t k2;
	uint64_t      value;
} node_t;

int countcmps = 0;

/* my comparison function compares keys */
int mycompare(ts_algo_key_t  old,
              ts_algo_key_t  new)
{
	countcmps++;
	if (old > new) return ts_algo_cmp_less;
	if (old < new) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

/* node comparison function compares nodes */
int nodecompare(node_t *old,
                node_t *new)
{
	countcmps++;
	if (old->k1 > new->k1) return ts_algo_cmp_less;
	if (old->k1 < new->k1) return ts_algo_cmp_greater;
	if (old->k2 > new->k2) return ts_algo_cmp_less;
	if (old->k2 < new->k2) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

/* showNode */
void showNode(node_t *n) {}

/* node update */
void onUpdate(node_t *on, node_t *nn) {
	free(nn);
}

/* do nothing */
void noUpdate(node_t *on, node_t *nn) {}

/* node destroy destroys nodes */
void onDestroy(node_t **node) {
	if (*node != NULL) {
		free(*node); *node=NULL;
	}
}

/* do nothing */
void noDestroy(node_t **node) {}

/* insert preallocated nodes */
char inserttest1(int it) {
	uint64_t i,j;
	ts_algo_tree_t *tree;
	timestamp_t t1,t2;
	uint64_t d = 0;
	progress_t p;

	init_progress(&p,stdout,it);
	countcmps=0;
	for (j=0;j<it;j++) {
		tree = ts_algo_tree_new(
		       (ts_algo_compare_t)&mycompare,
	               (ts_algo_show_t)&showNode,
	               (ts_algo_update_t)&onUpdate,
	               (ts_algo_delete_t)&noDestroy,
	               (ts_algo_delete_t)&noDestroy);

		if (tree == NULL) return 0;
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			return 0;
		}
		for (i=1;i<ELEMENTS;i++) {
			if (ts_algo_tree_insert(tree,(void*)i) != TS_ALGO_OK) {
				printf("cannot insert\n");
				return 0;
			}
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			return 0;
		}
		d += timediff(&t2,&t1);
		ts_algo_tree_destroy(tree); free(tree);
		update_progress(&p,(int) j);
	}
	close_progress(&p);printf("\n");
	d /= 1000*it;

	printf("%d inserts without allocation: %llu usecs\n", ELEMENTS, 
	      (unsigned long long)d);
	printf("average number of compares is %d\n",countcmps/(it*ELEMENTS));

	return 1;
}

/* insert dynamcially allocated nodes */
char inserttest2(int it) {
	int i,j;
	ts_algo_tree_t *tree;
	node_t             *node;
	timestamp_t t1,t2;
	uint64_t d = 0;
	progress_t p;

	init_progress(&p,stdout,it);
	countcmps=0;
	for (j=0;j<it;j++) {
		tree = ts_algo_tree_new(
		       (ts_algo_compare_t)&mycompare,
	               (ts_algo_show_t)&showNode,
	               (ts_algo_update_t)&onUpdate,
	               (ts_algo_delete_t)&onDestroy,
	               (ts_algo_delete_t)&onDestroy);
		if (tree == NULL) return 0;
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			return 0;
		}
		for (i=0;i<ELEMENTS;i++) {
			node = malloc(sizeof(node_t));
			if (node == NULL) return 0;
			node->k1 = keys[rand()%ELEMENTS];
			node->k2 = keys[rand()%ELEMENTS];
			node->value = randomUnsigned(1,1000);
			if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) return 0;
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			return 0;
		}
		d += timediff(&t2,&t1);
		ts_algo_tree_destroy(tree); free(tree);
		update_progress(&p,j);
	}
	close_progress(&p);printf("\n");
	d /= 1000*it;

	printf("%d inserts with allocation: %llu usecs\n", ELEMENTS,
          (unsigned long long) d);
	printf("average number of compares is %d\n",countcmps/(it*ELEMENTS));

	return 1;
}

/* search with hits (is faster on average) */
char findtest1(int it) {
	int i,j;
	timestamp_t t1,t2;
	uint64_t d = 0;
	ts_algo_tree_t *tree;
	ts_algo_key_t       what;
	ts_algo_key_t       node;
	progress_t p;

	tree = ts_algo_tree_new(
		       (ts_algo_compare_t)&mycompare,
	               (ts_algo_show_t)&showNode,
	               (ts_algo_update_t)&noUpdate,
	               (ts_algo_delete_t)&noDestroy,
	               (ts_algo_delete_t)&noDestroy);
	if (tree == NULL) return 0;
	for (i=0;i<ELEMENTS;i++) {
		node = keys[i];
		if (ts_algo_tree_insert(tree,(void*)node) != TS_ALGO_OK) 
			return 0;
	}
	
	init_progress(&p,stdout,it);
	countcmps=0;
	for (j=0;j<it;j++) {
		what=keys[rand()%ELEMENTS];
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		for (i=0;i<ELEMENTS;i++) {
			node = (ts_algo_key_t)ts_algo_tree_find(
			                      tree,(void*)what);
			if (node == 0) return FALSE;
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		d += timediff(&t2,&t1);
		update_progress(&p,j);
	}
	close_progress(&p);printf("\n");
	d /= (1000*it);
	printf("%d successful searches: %llu usecs\n", ELEMENTS, 
	      (unsigned long long)d);
	printf("average number of compares is %d\n", countcmps/(it*ELEMENTS));
	ts_algo_tree_destroy(tree); free(tree); 
	return 1;
}

/* search without hits (is slower on average) */
char findtest2(int it) {
	int i,j;
	timestamp_t t1,t2;
	uint64_t d = 0;
	ts_algo_tree_t  *tree;
	ts_algo_key_t    what;
	ts_algo_key_t    node;
	progress_t p;

	tree = ts_algo_tree_new(
		       (ts_algo_compare_t)&mycompare,
	               (ts_algo_show_t)&showNode,
	               (ts_algo_update_t)&noUpdate,
	               (ts_algo_delete_t)&noDestroy,
	               (ts_algo_delete_t)&noDestroy);
	if (tree == NULL) return 0;
	for (i=0;i<ELEMENTS;i++) {
		node = keys[i];
		if (ts_algo_tree_insert(tree,(void*)node) != TS_ALGO_OK) 
			return 0;
	}
	
	init_progress(&p,stdout,it);
	countcmps=0;
	for (j=0;j<it;j++) {
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		for (i=0;i<ELEMENTS;i++) {
			what = i+10*ELEMENTS;
			node = (ts_algo_key_t)ts_algo_tree_find(
			                      tree,(void*)what);
			if (node != 0) {
				printf("%llu found!\n",
				      (unsigned long long)what);
				return 0;
			}
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		d += timediff(&t2,&t1);
		update_progress(&p,j);
	}
	close_progress(&p);printf("\n");
	d /= 1000*it;
	printf("%d failed searches: %llu usecs\n", ELEMENTS, 
	      (unsigned long long)d);
	printf("average number of compares is %d\n", countcmps/(it*ELEMENTS));
	ts_algo_tree_destroy(tree); free(tree); 
	return 1;
}

int main () {
	int i;
	int it=51;

	init_rand();

	for (i=0;i<ELEMENTS;i++) {
		keys[i] = randomUnsigned(1,8*ELEMENTS);
	}

	if (!inserttest1(it)) {
		printf("insert1 failed!\n");
		return EXIT_FAILURE;
	}
	if (!inserttest2(it)) {
		printf("insert2 failed!\n");
		return EXIT_FAILURE;
	}
	if (!findtest1(it)) {
		printf("find1 failed!\n");
		return EXIT_FAILURE;
	}
	if (!findtest2(it)) {
		printf("find2 failed!\n");
		return EXIT_FAILURE;
	}
}
