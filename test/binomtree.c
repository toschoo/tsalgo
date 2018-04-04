/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * Store all ways to choose k out of n in an AVL tree.
 * key: (n,k), data: k values
 * ========================================================================
 */
#include <tsalgo/tree.h>
#include <tsalgo/list.h>
#include <tsalgo/random.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------
 * A set with the elements as indices
 * ------------------------------------------------------------------------
 */
typedef struct {
	size_t    size;
	uint32_t *elements;
} set_t;

/* ------------------------------------------------------------------------
 * The set is initialised with the elements in "natural" order,
 * i.e. from 0..size-1
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t initset(set_t *s, size_t size) {
	size_t i;

	s->elements = malloc(size*sizeof(uint32_t));
	if (s->elements == NULL) return TS_ALGO_ERR;
	s->size = size;
	for (i=0;i<size;i++) {
		s->elements[i] = i;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Destroy the set 
 * ------------------------------------------------------------------------
 */
void destroyset(set_t *s) {
	if (s->elements != NULL) {
		free(s->elements); s->elements=NULL;
	}
}

/* ------------------------------------------------------------------------
 * Compute the factorial
 * Note that, for any k <= 1, the result is 1
 * ------------------------------------------------------------------------
 */
uint32_t fac(uint32_t k) {
	uint32_t i;
	uint32_t r = k;
	if (r<=1) return 1;
	for(i=2;i<k;i++) r*=i;
	return r;
}

/* ------------------------------------------------------------------------
 * Compute the factorial to the k falling
 * For any k <= 1, the result is 1
 * ------------------------------------------------------------------------
 */
uint32_t toTheKF(uint32_t n,uint32_t k) {
	uint32_t i;
	uint32_t r = 1;
	uint32_t h = n;
	for(i=0;i<k;i++) {
		r*=h;h--;
	}
	return r;
}

/* ------------------------------------------------------------------------
 * Compute the binomial coefficient n choose k
 * ------------------------------------------------------------------------
 */
uint32_t choose(uint32_t n, uint32_t k) {
	if (k <  0) return 0;
	if (k >  n) return 0;
	if (k == 0) return 1;
	if (k == n) return 1;
	return (toTheKF(n,k)/fac(k));
}

/* ------------------------------------------------------------------------
 * Copy all elements in set src
 * whose indices correspond to one of the digits 
 * that are 1 in the binary representation of d
 * to set trg;
 * this method to create all combinations of k out of n
 * is not efficient, but it is very nice...
 * ------------------------------------------------------------------------
 */
void getK(set_t *src, set_t *trg, uint32_t d) {
	uint32_t i=0,j=0,m=1;

	for (i=0;j<trg->size;i++) {
		if (m & d) {
			trg->elements[j] = src->elements[i]; j++;
		}
		m*=2;
	}
}

/* ------------------------------------------------------------------------
 * check if number has k non-zero bits 
 * ------------------------------------------------------------------------
 */
ts_algo_bool_t hasBits(uint64_t z, uint32_t k) {
	uint32_t i,c=0;

	for(i=1;i<=z;i*=2) {
		if (z&i) c++;
		if (c>k) return FALSE;
	}
	return (c==k);
}

/* ------------------------------------------------------------------------
 * get list of numbers with k bits == 1.
 * This is extremely inefficient, of course,
 * but it is a nice method.
 * ------------------------------------------------------------------------
 */
uint64_t *getNumbers(uint32_t n, uint32_t k) {
	uint64_t  x;
	uint64_t  m = pow(2,n);
	uint64_t *r;
	uint32_t  b;
	uint32_t  i=0;

	b = choose(n,k);
	r = malloc(b*sizeof(uint64_t));
	if (r == NULL) return NULL;

	for (x=1;x<m;x++) {
		if (hasBits(x,k)) {
			r[i]=x;i++;
		}
	}
	return r;
}

/* ------------------------------------------------------------------------
 * The node type:
 * key  : n,k
 * value: list of possible combinations of n choose k
 * ------------------------------------------------------------------------
 */
typedef struct {
	uint32_t       n;
	uint32_t       k;
	ts_algo_list_t combis;
} combi_t;

/* ------------------------------------------------------------------------
 * Allocate a node, set the key and initialise the list
 * ------------------------------------------------------------------------
 */
combi_t *makenode(uint32_t n, uint32_t k) {
	combi_t *node;

	node = malloc(sizeof(combi_t));
	if (node == NULL) return NULL;
	node->n = n;
	node->k = k;
	ts_algo_list_init(&node->combis);
	return node;
}

/* ------------------------------------------------------------------------
 * Compare two nodes
 * ------------------------------------------------------------------------
 */
ts_algo_cmp_t compare(void *ignore, combi_t *left, combi_t *right) {
	if (left->n < right->n) return ts_algo_cmp_less;
	if (left->n > right->n) return ts_algo_cmp_greater;
	if (left->k < right->k) return ts_algo_cmp_less;
	if (left->k > right->k) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

/* ------------------------------------------------------------------------
 * On collision:
 * - add the head of the list of the new node 
 *                to the list of the old node
 * - free all resources held by the new node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t addCombi(void *ignore, combi_t *oldN, combi_t *newN) {
	set_t *s;

	if (newN == NULL) return TS_ALGO_OK;
	if (newN->combis.head == NULL) return TS_ALGO_OK;
	s = newN->combis.head->cont;
	ts_algo_list_destroy(&newN->combis); free(newN);
	if (ts_algo_list_append(&oldN->combis,s) != TS_ALGO_OK)
	{
		destroyset(s); free(s);
		return TS_ALGO_ERR;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * On destroy:
 * - destroy and free all sets in the list
 * - destroy and free the the list itself
 * - free the node
 * ------------------------------------------------------------------------
 */
void destroy(void *ignore, combi_t **p) {
	ts_algo_list_node_t *runner;
	set_t *c;

	if (*p != NULL) {
		for (runner=(*p)->combis.head;
		     runner!=NULL;
		     runner=runner->nxt)
		{
			c=runner->cont;
			destroyset(c);free(c);
		}
		ts_algo_list_destroy(&(*p)->combis);
		free(*p); *p=NULL;
	}
}

/* ------------------------------------------------------------------------
 * On delete: ignore
 * ------------------------------------------------------------------------
 */
void ignore(void *ignore, combi_t **p) {}

/* ------------------------------------------------------------------------
 * Show:
 * - show the keys
 * - show a list of all possible combinations
 * ------------------------------------------------------------------------
 */
void show(combi_t *p) {
	uint32_t i,z=0;
	set_t    *c;
	ts_algo_list_node_t *runner;

	printf("combinations of %u choose %u: \n", p->n, p->k);
	for (runner=p->combis.head;
	     runner!=NULL;
	     runner=runner->nxt)
	{
		if (z==0) printf("\t");
		else if (z%5==0) printf("\n\t");
		else printf("||");
		z++; c=runner->cont;
		for(i=0;i<c->size;i++) {
			if (i!=0) printf(",");
			printf("%u", c->elements[i]);
		}
	}
	printf("\n");
}

/* ------------------------------------------------------------------------
 * insert all combinations of k out of n
 * for random pairs (n,k) (that we have not yet inserted!)
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t randomChoose(ts_algo_tree_t *tree) {
	uint32_t n,k,b,i;
	uint64_t *bs;
	set_t    ns,*ks;
	combi_t  *node,snode;

	do {
		do n=rand()%10; while(n==0); k=rand()%n;

		printf("%u choose %u\n", n,k);

		snode.n = n;
		snode.k = k;
		node = ts_algo_tree_find(tree,&snode);
	} while (node != NULL);

	b = choose(n,k);
	
	bs = getNumbers(n,k);
	if (bs == NULL) return TS_ALGO_ERR;

	if (initset(&ns,n) != TS_ALGO_OK) {
		fprintf(stderr,"cannot init k-set\n");
		free(bs);
		return TS_ALGO_ERR;
	}
	for(i=0;i<b;i++) {
		ks = malloc(sizeof(set_t));
		if (ks == NULL) {
			free(bs); destroyset(&ns);
			return TS_ALGO_ERR;
		}
		if (initset(ks,k) != TS_ALGO_OK) {
			fprintf(stderr,"cannot init k-set\n");
			free(bs); destroyset(&ns);
			destroyset(ks); free(ks);
			return TS_ALGO_ERR;
		}
		getK(&ns,ks,bs[i]);
		node = makenode(n,k);
		if (node == NULL) {
			free(bs); destroyset(&ns);
			destroyset(ks); free(ks);
			return TS_ALGO_ERR;
		}
		/* Note that we add combi by combi, i.e.
		 *      there is only one element in the list
		 *      we insert into the tree.
		 *      Compare this to addCombi above!
		 */
		if (ts_algo_list_append(&node->combis,ks) != TS_ALGO_OK)
		{
			free(bs); destroyset(&ns);
			destroyset(ks); free(ks);
			return TS_ALGO_ERR;
		}
		if (ts_algo_tree_insert(tree,node) != TS_ALGO_OK) {
			free(bs); destroyset(&ns);
			destroyset(ks); free(ks);
			return TS_ALGO_ERR;
		}
	}
	free(bs); destroyset(&ns);
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * And we are almost done...
 * ------------------------------------------------------------------------
 */
int main() {
	int i;
	ts_algo_tree_t coeffs;

	/* init tree */
	if (ts_algo_tree_init(&coeffs,
	                      (ts_algo_comprsc_t)&compare,
	                      (ts_algo_show_t)&show,
	                      (ts_algo_update_t)&addCombi,
	                      (ts_algo_delete_t)&ignore,
	                      (ts_algo_delete_t)&destroy) != TS_ALGO_OK)
	{
		fprintf(stderr,"cannot init tree\n");
		return EXIT_FAILURE;
	}
	/* insert 10 coefficients */
	init_rand();
	for(i=0;i<10;i++) {
		if (randomChoose(&coeffs) != TS_ALGO_OK)
		{
			ts_algo_tree_destroy(&coeffs);
			fprintf(stderr,"cannot insert into tree\n");
			return EXIT_FAILURE;
		}
	}
	/* show tree */
	ts_algo_tree_show(&coeffs);

	/* clean up */
	ts_algo_tree_destroy(&coeffs);
	return EXIT_SUCCESS;
}
