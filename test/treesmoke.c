#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <tsalgo/tree.h>

typedef struct {
	uint64_t k1;
	uint64_t k2;
	uint64_t k3;
	uint64_t k4;
} mynode_t;

static int compareNodes(void *ignore,
                        mynode_t *n1,
                        mynode_t *n2)
{
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


static void showNode(mynode_t *node) {
	printf("%04lu-%04lu-%04lu-%04lu\n", 
	       node->k1,node->k2,node->k3,node->k4);
}

static ts_algo_rc_t onUpdate(void     *ignore,
                             mynode_t *oldNode,
                             mynode_t *newNode)
{
	free(newNode); return TS_ALGO_OK;
}

static void onDelete(void    *ignore,
                     mynode_t **node) {
	if (*node != NULL) {
		free(*node); *node=NULL;
	}
}

void simpletest() {
	ts_algo_tree_t *tree;
	mynode_t       *n;
	uint64_t i;

	tree = ts_algo_tree_new(
	          (ts_algo_comprsc_t)&compareNodes,
	          (ts_algo_show_t)&showNode,
	          (ts_algo_update_t)&onUpdate,
	          (ts_algo_delete_t)&onDelete,
	          (ts_algo_delete_t)&onDelete);
	if (tree == NULL) {
		return;
	}

	for (i=1;i<=256;i++) {
		n = malloc(sizeof(mynode_t));
		if (n == NULL) {
			printf("cannot allocate memory\n"); 
			return;
		}
		n->k1 = i;
		n->k2 = 0;
		n->k3 = 0;
		n->k4 = 0;
		if (ts_algo_tree_insert(tree,n) != TS_ALGO_OK) {
			printf("Cannot insert\n"); 
			return;
		}
	}
	ts_algo_tree_show(tree);
	printf("height: %d\n", ts_algo_tree_height(tree));
	if (ts_algo_tree_balanced(tree)) {
		printf("tree is balanced\n");
	} else {
		printf("tree is *not* balanced\n");
	}
	ts_algo_tree_destroy(tree); free(tree);
}

mynode_t *mknode(uint64_t key) {
	mynode_t *n;
	n = malloc(sizeof(mynode_t));
	if (n == NULL) {
		printf("cannot alloc node\n");
		return NULL;
	}
	n->k1 = key;
	n->k2 = 0;
	n->k3 = 0;
	n->k4 = 0;
	return n;
}

void showNodeK1(ts_algo_tree_node_t *t) {
	mynode_t *tmp;
	if (t == NULL) return;
	tmp = t->cont;
	printf("%03lu", tmp->k1);
}

void show(ts_algo_tree_t *tree) {
	ts_algo_list_t list;
	ts_algo_list_node_t *runner;
	int i,j,k,n;

	int h = ts_algo_tree_height(tree);
	fprintf(stderr, "height: %d\n", h);

	n = pow(2,h-2);
	for(i=0;i<h;i++) {

		ts_algo_list_init(&list);
		if (ts_algo_tree_grabGeneration(tree, &list, i) != TS_ALGO_OK) {
			printf("error in grabbing generation %d!\n", i);
			return;
		}
		k=n>0?n*4-1:1;
		for(runner=list.head;runner!=NULL;runner=runner->nxt) {
			for(j=0;j<k;j++) printf(" ");
			uint64_t *key = runner->cont;
			if (key == NULL) printf("NIL"); 
			else printf("%03lu", *key);
			for(j=1;j<k;j++) printf(" ");
		}
		printf("\n");
		n /= 2;
		ts_algo_list_destroy(&list);
	}
}

void wirth() {
	ts_algo_tree_t *tree;
	mynode_t       *n,*tmp;
	char b;

	/* 1. */
	tree = ts_algo_tree_new(
	          (ts_algo_comprsc_t)&compareNodes,
	          (ts_algo_show_t)&showNode,
	          (ts_algo_update_t)&onUpdate,
	          (ts_algo_delete_t)&onDelete,
	          (ts_algo_delete_t)&onDelete);

	printf("inserting 40\n");
	n = mknode(40);
	ts_algo_tree_insert(tree,n);

	printf("inserting 50\n");
	n = mknode(50);
	ts_algo_tree_insert(tree,n);

	printf("inserting 70\n");
	n = mknode(70);
	ts_algo_tree_insert(tree,n);

	printf("inserting 20\n");
	n = mknode(20);
	ts_algo_tree_insert(tree,n);

	printf("inserting 10\n");
	n = mknode(10);
	ts_algo_tree_insert(tree,n);

	printf("inserting 30\n");
	n = mknode(30);
	ts_algo_tree_insert(tree,n);

	/*
	printf("inserting 5\n");
	n = mknode(5);
	ts_algo_tree_insert(tree,n);

	printf("inserting 13\n");
	n = mknode(13);
	ts_algo_tree_insert(tree,n);
	*/

	printf("inserting 60\n");
	n = mknode(60);
	ts_algo_tree_insert(tree,n);

	printf("inserting 80\n");
	n = mknode(80);
	ts_algo_tree_insert(tree,n);
	
	printf("inserting 5\n");
	n = mknode(5);
	ts_algo_tree_insert(tree,n);

	printf("inserting 15\n");
	n = mknode(15);
	ts_algo_tree_insert(tree,n);

	printf("inserting 25\n");
	n = mknode(25);
	ts_algo_tree_insert(tree,n);

	printf("inserting 35\n");
	n = mknode(35);
	ts_algo_tree_insert(tree,n);

	printf("inserting 65\n");
	n = mknode(65);
	ts_algo_tree_insert(tree,n);

	printf("inserting 45\n");
	n = mknode(45);
	ts_algo_tree_insert(tree,n);

	printf("inserting 55\n");
	n = mknode(55);
	ts_algo_tree_insert(tree,n);

	/*
	printf("inserting 90\n");
	n = mknode(90);
	ts_algo_tree_insert(tree,n);
	*/

	tmp = tree->tree->cont;
	printf("balance %lu: %d\n", 
	       (unsigned long)tmp->k1,tree->tree->bal);
	if (tree->tree->right != NULL) {
		tmp = tree->tree->right->cont;
		printf("balance %lu: %d\n", 
		       (unsigned long)tmp->k1,
		       tree->tree->right->bal);
	}
	if (tree->tree->left != NULL) {
		tmp = tree->tree->left->cont;
		printf("balance %lu: %d\n", (unsigned long)tmp->k1,
		                            tree->tree->left->bal);
	}
	if (tree->tree->right->right != NULL) {
		tmp = tree->tree->right->right->cont;
		printf("balance %lu, %d\n", (unsigned long)tmp->k1,
		                            tree->tree->right->right->bal);
	}
	if (tree->tree->right->left != NULL) {
		tmp = tree->tree->right->left->cont;
		printf("balance %lu, %d\n", (unsigned long)tmp->k1,
		                            tree->tree->right->left->bal);
	}

	show(tree);
	printf("height: %d\n", ts_algo_tree_height(tree));

	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 45\n");
	printf(  "===========\n");
	n = mknode(45);
	ts_algo_tree_delete(tree,n);
	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);
	tmp = tree->tree->right->left->cont;
	b = tree->tree->right->left->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 50\n");
	printf(  "===========\n");
	n = mknode(50);
	ts_algo_tree_delete(tree,n);
	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);
	tmp = tree->tree->right->left->cont;
	b = tree->tree->right->left->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 60\n");
	printf(  "===========\n");
	n = mknode(60);
	ts_algo_tree_delete(tree,n);
	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);
	tmp = tree->tree->right->right->cont;
	b = tree->tree->right->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ninserting 85\n");
	printf(  "============\n");
	n = mknode(85);
	ts_algo_tree_insert(tree,n);
	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	tmp = tree->tree->right->left->cont;
	b = tree->tree->right->left->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	tmp = tree->tree->right->right->cont;
	b = tree->tree->right->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 55\n");
	printf(  "===========\n");
	n = mknode(55);
	ts_algo_tree_delete(tree,n);
	show(tree);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	tmp = tree->tree->right->left->cont;
	b = tree->tree->right->left->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 80\n");
	printf(  "===========\n");
	n = mknode(80);
	ts_algo_tree_delete(tree,n);
	show(tree);

	tmp = tree->tree->cont;
	b = tree->tree->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	tmp = tree->tree->right->cont;
	b = tree->tree->right->bal;
	printf("balance %lu, %d\n", (unsigned long)tmp->k1, b);

	printf("\ndeleting 65\n");
	printf(  "===========\n");
	n = mknode(65);
	ts_algo_tree_delete(tree,n);
	show(tree);

	printf("\ndeleting 20\n");
	printf(  "===========\n");
	n = mknode(20);
	ts_algo_tree_delete(tree,n);
	show(tree);

	ts_algo_tree_destroy(tree); free(tree);

}

int main () {
	simpletest(); 
	wirth();
	return EXIT_SUCCESS;
}
