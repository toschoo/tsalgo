/* ========================================================================
 * (c) Tobias Schoofs, 2015 -- 2016
 * ========================================================================
 * Print a tree on the terminal.
 * ========================================================================
 * The program receives an insert list and (optionally) a delete list.
 * It inserts the insert list into a tree and prints it.
 * Then it deletes the elements on the delete list 
 * and prints the tree again.
 * Reports an error, if the tree is not balanced.
 * The lists must consists of natural numbers of at most 3 digits.
 * ========================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tsalgo/tree.h>

/* comparison callback */
static int compareNodes(void *ignore,
                        ts_algo_key_t *k1,
                        ts_algo_key_t *k2)
{
	if (*k1 < *k2) return ts_algo_cmp_less;
	if (*k1 > *k2) return ts_algo_cmp_greater;

	return ts_algo_cmp_equal;
}

/* show callback */
static void showNode(ts_algo_key_t *k1) {
	if (k1 == NULL) return;
	fprintf(stderr, "%03lu", *k1);
}

/* we free on update */
static ts_algo_rc_t onUpdate(ts_algo_key_t *ok,
                             ts_algo_key_t *nk)
{
	free(nk); return TS_ALGO_OK;
}

/* we free on delete */
static void onDelete(ts_algo_key_t **k) {
	if (*k != NULL) {
		free(*k); *k=NULL;
	}
}

/* show the tree */
void show(ts_algo_tree_t *tree) {
	ts_algo_list_t list;
	ts_algo_list_node_t *runner;
	int i,j,k,n;

	int h = ts_algo_tree_height(tree);
	fprintf(stderr, "height: %d\n", h);

	n = pow(2,h-2); /* number of elements per generation */
	/* for all generations */
	for(i=0;i<h;i++) {

		/* grab the current generation */
		ts_algo_list_init(&list);
		if (ts_algo_tree_grabGeneration(tree, &list, i) != TS_ALGO_OK) {
			printf("error in grabbing generation %d!\n", i);
			return;
		}
		/* determine the maximum number of elements */
		k=n>0?n*4-1:1;

		/* print the current generation */
		for(runner=list.head;runner!=NULL;runner=runner->nxt) {
			for(j=0;j<k;j++) printf(" ");
			ts_algo_key_t *key = runner->cont;
			if (key == NULL) printf("NIL"); 
			else printf("%03lu", *key);
			for(j=1;j<k;j++) printf(" ");
		}
		printf("\n");
		n /= 2;
		ts_algo_list_destroy(&list);
	}
}

/* ------------------------------------------------------------------------
 * build the tree 
 * --------------
 * - creates the tree
 * - inserts all elements on the insert list
 * - prints the tree
 * - deletes all elements on the delete list
 * - prints the tree again
 * - destroys the tree
 * - if the tree is not balance, an error is reported.
 * ------------------------------------------------------------------------
 */
int buildTree(ts_algo_list_t *ins,
              ts_algo_list_t *del) 
{
	ts_algo_list_node_t *runner;
	ts_algo_tree_t        *tree;
	int rc = 0;

	tree = ts_algo_tree_new(
	          (ts_algo_comprsc_t)&compareNodes,
	          (ts_algo_show_t)&showNode,
	          (ts_algo_update_t)&onUpdate,
	          (ts_algo_delete_t)&onDelete,
	          (ts_algo_delete_t)&onDelete);

	if (tree == NULL) {
		fprintf(stderr, "cannot create tree\n");
		return -1;
	}

	for(runner=ins->head;runner!=NULL;runner=runner->nxt) {
		ts_algo_key_t *key = runner->cont;

		printf("inserting %lu\n", *key);
		if (ts_algo_tree_insert(tree,key) != TS_ALGO_OK) {
			fprintf(stderr, "cannot insert %lu into tree", *key);
			return -1;
		}
	}
	printf("=====================\n");
	show(tree); printf("\n"); ts_algo_tree_showbal(tree);
	printf("=====================\n");

	for(runner=del->head;runner!=NULL;runner=runner->nxt) {
		ts_algo_key_t *key = runner->cont;

		printf("deleting %lu\n", *key);
		ts_algo_tree_delete(tree,key);
	}
	if (del->len > 0) {
		printf("=====================\n");
		show(tree); printf("\n"); ts_algo_tree_showbal(tree);
		printf("=====================\n");
	}
	if (!ts_algo_tree_balanced(tree)) rc = -1;

	ts_algo_tree_destroy(tree); free(tree);
	return rc;

}

/* convert a string encoding a list of natural numbers < 1000 into a list */
int getList(char *str, ts_algo_list_t *list) {
	char buf[4];
	ts_algo_key_t *k;
	size_t s;
	int i,x,j=0;

	s = strnlen(str, 4096);
	if (s >= 4095) {
		fprintf(stderr, "list too long\n");
		return -1;
	}
	buf[j] = 0;
	for(i=0;i<=s;i++) {
		if (j>3) {
			fprintf(stderr, "key too long\n");
			return -1;
		}
		if (str[i] == ',' || i == s) {
			if (j < 1) {
				fprintf(stderr, "empty key\n");
				return -1;
			}
			buf[j] = 0;
			for(x=0;x<j;x++) {
				if (buf[x] < '0' || x > '9') {
					fprintf(stderr, "not numeric\n");
					return -1;
				}
			}
			k = malloc(sizeof(ts_algo_key_t));
			if (k == NULL) {
				fprintf(stderr, "out of memory\n");
				return -1;
			}
			*k = atoi(buf);
			if (ts_algo_list_append(list, k) != TS_ALGO_OK) {
				fprintf(stderr, "out of memory\n");
				free(k);
				return -1;
			}
			j=0;
		} else {
			buf[j] = str[i]; j++;
		}
	}
	return 0;
}

/* give help */
void usage(char *prog) {
	fprintf(stderr, "%s <insert list> [<delete list>]\n", prog);
	fprintf(stderr, "\tlist: comma-separated list of natural numbers, e.g.\n");
	fprintf(stderr, "\t1,2,3,4,5\n");
}

/* main */
int main(int argc, char **argv) {
	ts_algo_list_t ins, dels;
	int rc = EXIT_SUCCESS;

	if (argc < 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	ts_algo_list_init(&ins);
	ts_algo_list_init(&dels);

	if (getList(argv[1], &ins) != 0) {
		fprintf(stderr, "cannot read insert list\n");
		return EXIT_FAILURE;
	}
	if (argc > 2) {
		if (getList(argv[2], &dels) != 0) {
			fprintf(stderr, "cannot read delete list\n");
			return EXIT_FAILURE;
		}
	}
	if (buildTree(&ins, &dels) != 0) {
		rc = EXIT_FAILURE;
	}

	ts_algo_list_destroy(&ins);
	ts_algo_list_destroy(&dels);
	
	return rc;
}
