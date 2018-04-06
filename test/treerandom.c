/* ========================================================================
 * Test balanced binary tree
 * -------------------------
 * (c) Tobias Schoofs, 2011 -- 2017
 * ========================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <tsalgo/random.h>
#include <tsalgo/tree.h>
#include <progress.h>

#define ELEMENTS 4096

/* node to store in "nub" 
 * which is a list of unique elements
 * that serves to check on the number
 * of elements in the tree
 */
typedef struct {
	uint64_t k;
	char          x;
} nub_t;

/* initialise the nub
 * note that we use the value '0'
 * to find the end of the list
 */
void init_nub(nub_t *nub) {
	int i;
	for (i=0;i<ELEMENTS;i++) {
		nub[i].k = 0;
		nub[i].x = 0;
	}
}

/* insert an element */
void insert(nub_t *nub, uint64_t val) {
	int i;
	for (i=0;i<ELEMENTS;i++) {
		if (nub[i].k == 0) {
			nub[i].k = val; 
			nub[i].x = 1;
			break;
		}
		if (nub[i].k == val) break;
	}
}

/* count all elements */
int count_nub(nub_t *nub) {
	int i;
	for (i=0;i<ELEMENTS;i++) {
		if (nub[i].k == 0) break;
	}
	return i;
}

/* count only those that are set */
int count_setnub(nub_t *nub) {
	int i,j=0;
	for (i=0;i<ELEMENTS;i++) {
		if (nub[i].k == 0) break;
		if (nub[i].x) j++;
	}
	return j;
}

/* set a given element */
void set_nub(nub_t *nub, uint64_t val, char x) {
	int i;
	for (i=0;i<ELEMENTS;i++) {
		if (nub[i].k == 0) break;
		if (nub[i].k == val) {
			nub[i].x = x; break;
		}
	}
}

/* node to be inserted in the tree */
typedef struct {
	uint64_t k1;
	uint64_t k2;
	uint64_t k3;
	uint64_t k4;
} mynode_t;

/* how to compare such a node */
static int compareNodes(void *ignore, mynode_t *n1, mynode_t *n2) {
	if (n1->k1 < n2->k1) return ts_algo_cmp_less;
	if (n1->k1 > n2->k1) return ts_algo_cmp_greater;

	if (n1->k2 < n2->k2) return ts_algo_cmp_less;
	if (n1->k2 > n2->k2) return ts_algo_cmp_greater;

	if (n1->k3 < n2->k3) return ts_algo_cmp_less;
	if (n1->k3 > n2->k3) return ts_algo_cmp_greater;

	if (n1->k4 < n2->k4) return ts_algo_cmp_less;
	if (n1->k4 > n2->k4) return ts_algo_cmp_greater;

	return ts_algo_cmp_equal;
}

/* how to show such a node */
static void showNode(mynode_t *node) {
	printf("%04lu-%04lu-%04lu-%04lu\n", 
	       node->k1,node->k2,node->k3,node->k4);
}

/* how to update */
static ts_algo_rc_t onUpdate(void     *ignore,
                             mynode_t *oldNode, 
                             mynode_t *newNode) 
{
	free(newNode); /* we use collision to cleanup memory */
	return TS_ALGO_OK;
}

/* how to destroy */
static void onDestroy(void *ignore, mynode_t **node)
{
	if (*node == NULL) return;
	free(*node); *node = NULL;
}

/* how to delete */
static void onDelete(void *ignore, mynode_t **node)
{
	if (*node == NULL) return;
	free(*node); *node = NULL;
}

/* test insert */
char tstinsert() {
	int i;
	int h,n,nt;
	char r;
	uint64_t keys[ELEMENTS];
	nub_t          nub[ELEMENTS];
	ts_algo_tree_t *tree;
	mynode_t *node;

	init_nub(nub);
	for (i=0;i<ELEMENTS;i++) {
		keys[i] = randomUnsigned(1,8*ELEMENTS);
		insert(nub,keys[i]);
	}
	tree = ts_algo_tree_new(
	         (ts_algo_comprsc_t)&compareNodes,
	         (ts_algo_show_t)&showNode,
	         (ts_algo_update_t)&onUpdate,
	         (ts_algo_delete_t)&onDelete,
	         (ts_algo_delete_t)&onDestroy);
	if (tree == NULL) return 0;
	for (i=0;i<ELEMENTS;i++) {
		/* duplicates are freed in onUpdate */
		node = malloc(sizeof(mynode_t));
		if (node == NULL) return 0;
		node->k1 = keys[i];
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) return 0;
	}

	r = 1;

	n  = count_nub(nub);
	nt = tree->count;
	if (nt != n) {
		printf("failed: count is %d, expected: %d\n", nt, n);
		r = 0;
	}
	if (r) {
		h = ts_algo_tree_height(tree);
		if (h > 1.5*ceil(log2(nt))) { /* height of AVL = 1.44log(n) */
			printf("failed: height is %d, expected: %f for %d elements\n", 
			       h, 1.5*ceil(log2(nt)),nt);
			r = 0;
		}
	}
	if (r) {
		r = ts_algo_tree_balanced(tree);
		if (!r) {
			printf("tree is not balanced!\n");
		}
	}
	ts_algo_tree_destroy(tree); free(tree);

	return r;
}

/* test insert and delete */
char tstinsdel() {
	int i;
	int h,n=0;
	char r;
	uint64_t keys[ELEMENTS];
	nub_t          nub[ELEMENTS];
	ts_algo_tree_t *tree;
	mynode_t *node;

	init_nub(nub);
	for (i=0;i<ELEMENTS;i++) {
		keys[i] = randomUnsigned(1,8*ELEMENTS);
		insert(nub,keys[i]);
	}
	tree = ts_algo_tree_new(
	         (ts_algo_comprsc_t)&compareNodes,
	         (ts_algo_show_t)&showNode,
	         (ts_algo_update_t)&onUpdate,
	         (ts_algo_delete_t)&onDelete,
	         (ts_algo_delete_t)&onDestroy);
	if (tree == NULL) return 0;
	for (i=0;i<ELEMENTS;i++) {
		/* duplicates are freed in onUpdate */
		node = malloc(sizeof(mynode_t));
		if (node == NULL) return 0;
		node->k1 = keys[i];
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) return 0;
	}
	for (i=ELEMENTS/2+1;i<ELEMENTS;i++) {
		unsigned int w = randomUnsigned(1,10);
		node = malloc(sizeof(mynode_t));
		if (node == NULL) return 0;
		node->k1 = keys[i];
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (w<=6) {
			if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) return 0;
			set_nub(nub, keys[i],1);
		} else {
			ts_algo_tree_delete(tree,node); free(node);
			set_nub(nub, keys[i],0);
		}
	}

	r = 1;

	n = count_setnub(nub);
	if (n != tree->count) {
		printf("number of elements wrong, expected: %d, tree: %d!\n",
			n, tree->count);
		r = 0;
	}

	if (r) {
		h = ts_algo_tree_height(tree);
		if (h > 2*ceil(log2(n))+1) {
			printf("failed: height is %d, expected: %f for %d elements\n", 
			       h, 2*ceil(log2(n))+1,n);
			r = 0;
		}
	}
	if (r) {
		r = ts_algo_tree_balanced(tree);
		if (!r) {
			printf("tree is not balanced!\n");
			printf("height is %d\n", ts_algo_tree_height(tree));
		}
	}
	ts_algo_tree_destroy(tree); free(tree);

	return r;
}

/* test update */
char tstupdate() {
	ts_algo_tree_t *tree;
	mynode_t       *node;
	char              rc;
	int                i;

	tree = ts_algo_tree_new(
	         (ts_algo_comprsc_t)&compareNodes,
	         (ts_algo_show_t)&showNode,
	         (ts_algo_update_t)&onUpdate,
	         (ts_algo_delete_t)&onDelete,
	         (ts_algo_delete_t)&onDestroy);

	for (i=0;i<100;i++) {
		node = malloc(sizeof(mynode_t));
		if (i < 90) node->k1 = i;
		else        node->k1 = 1;
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (ts_algo_tree_insert(tree,node)) return 0;
	}
	rc = (tree->count == 90);
	ts_algo_tree_destroy(tree); free(tree);
	return rc;
}

/* execute insert, delete, upate 'it' times */
char exectest(int it) {
	progress_t p;
	int i;

	init_progress(&p,stdout,it);
	for (i=0;i<it;i++) {
		if (!tstinsert()) {
			printf("\ninsert failed after %d tests\n", i);
			return 0;
		}
		if (!tstinsdel()) {
			printf("\ndelete failed after %d tests\n", i);
			return 0;
		}
		if (!tstupdate()) {
			printf("\nupdate failed after %d tests\n", i);
			return 0;
		}
		update_progress(&p,i);
	}
	close_progress(&p); printf("\n");
	return 1;
}

/* validate that the resulting list is sorted */
ts_algo_bool_t validate(ts_algo_list_t *list) {
	uint64_t *k1,*k2;
	ts_algo_list_node_t *runner;

	runner = list->head;
	if (runner != NULL) {
		k1 = list->head->cont;
		runner=runner->nxt;
	}
	while(runner != NULL) {
		k2 = runner->cont;
		if (*k1 > *k2) {
			printf("%llu > %llu\n", 
			      (unsigned long long)(*k1),
			      (unsigned long long)(*k2));
			return FALSE;
		}
		k1 = k2;
		runner = runner->nxt;
	}
	return TRUE;
}

/* test toList 
   TODO: test for completeness!
 */
char list_test() {
	int i;
	char r;
	uint64_t keys[ELEMENTS];
	nub_t         nub[ELEMENTS];
	ts_algo_tree_t  tree;
	ts_algo_list_t *list;
	mynode_t *node;

	init_nub(nub);
	for (i=0;i<ELEMENTS;i++) {
		keys[i] = randomUnsigned(1,8*ELEMENTS);
		insert(nub,keys[i]);
	}
	if (ts_algo_tree_init(&tree,
	         (ts_algo_comprsc_t)&compareNodes,
	         (ts_algo_show_t)&showNode, 
	         (ts_algo_update_t)&onUpdate,
	         (ts_algo_delete_t)&onDelete,
	         (ts_algo_delete_t)&onDestroy) != TS_ALGO_OK)
	{
		return 0;
	}
	for (i=0;i<ELEMENTS;i++) {
		node = malloc(sizeof(mynode_t));
		if (node == NULL) return 0;
		node->k1 = keys[i];
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (ts_algo_tree_insert(&tree,node) != TS_ALGO_OK) return 0;
	}
	list = ts_algo_tree_toList(&tree);
	if (list == NULL) return 0;

	r = validate(list);

	ts_algo_tree_destroy(&tree); 
	ts_algo_list_destroy(list); free(list);
	return r;
}

/* test finding elements in the tree */
char findtest() {
	int i,z;
	uint64_t keys[ELEMENTS];
	ts_algo_tree_t *tree;
	mynode_t       *node;
	mynode_t        what;
	for (i=0;i<ELEMENTS;i++) {
		keys[i] = randomUnsigned(1,8*ELEMENTS);
	}
	tree = ts_algo_tree_new(
	         (ts_algo_comprsc_t)&compareNodes,
	         (ts_algo_show_t)&showNode,
	         (ts_algo_update_t)&onUpdate,
	         (ts_algo_delete_t)&onDelete,
	         (ts_algo_delete_t)&onDestroy);
	if (tree == NULL) return 0;
	for (i=0;i<ELEMENTS;i++) {
		node = malloc(sizeof(mynode_t));
		if (node == NULL) return 0;
		node->k1 = keys[i];
		node->k2 = 0;
		node->k3 = 0;
		node->k4 = 0;
		if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) return 0;
	}
	what.k1 = 0;
	what.k2 = 0;
	what.k3 = 0;
	what.k4 = 0;
	for (z=0;z<2;z++) {
		for (i=0;i<ELEMENTS;i++) {
			what.k1 = keys[i];
			node = ts_algo_tree_find(tree,&what);
			if (z == 0 && node == NULL) return 0;
			if (z == 1 && node != NULL) return 0;
		}
		what.k2 = -1;
	}
	ts_algo_tree_destroy(tree); free(tree);
	return 1;
}

/* execute all tests */
int main () {
	int i;
	init_rand();

	printf("testing find\n");
	if (!findtest()) {
		printf("find failed!\n");
		return EXIT_FAILURE;
	}
	printf("testing insert, delete, update\n");
	if (!exectest(100)) {
		return EXIT_FAILURE;
	}
	printf("testing toList\n");
	for (i=0;i<10;i++) {
		if (!list_test()) {
			printf("list test failed\n");
			return EXIT_FAILURE;
		}
	}
	printf("all tests passed\n");
	return EXIT_SUCCESS;
}
