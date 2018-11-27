/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 * LRU Cache
 * ========================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <tsalgo/lru.h>

/* ------------------------------------------------------------------------
 * The internal structure of what is stored in the tree:
 * - the user content
 * - and a pointer to the list node
 * With this structure, we do not need to search for the node in the list.
 * ------------------------------------------------------------------------
 */
typedef struct {
	void *cont;
	ts_algo_list_node_t *lnode;
        char rsdnt;
} lru_node_t;

/* ------------------------------------------------------------------------
 * How to compare: we need to delegate
 * the comparison of the user content
 * to the user compare function.
 * ------------------------------------------------------------------------
 */
static ts_algo_cmp_t lruCompare(ts_algo_tree_t *tree,
                                lru_node_t     *one,
                                lru_node_t     *two) {
	return ((ts_algo_lru_t*)tree->rsc)->compare(
	                 NULL, one->cont, two->cont);
}

/* ------------------------------------------------------------------------
 * How to update: delegate to the user function.
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t lruUpdate(ts_algo_tree_t *tree,
                              lru_node_t     *oldN,
                              lru_node_t     *newN) {
	ts_algo_rc_t rc;
	rc = ((ts_algo_lru_t*)tree->rsc)->onUpdate(tree,oldN->cont,
	                                                newN->cont);
	if (rc == TS_ALGO_OK) free(newN);
	return rc;
}

/* ------------------------------------------------------------------------
 * How to delete: delegate to the user function.
 * ------------------------------------------------------------------------
 */
static void lruDelete(ts_algo_tree_t *tree,
                      lru_node_t    **n) {
	((ts_algo_lru_t*)tree->rsc)->onDelete(tree,&(*n)->cont);
	free(*n); *n=NULL;
}

/* ------------------------------------------------------------------------
 * How to destroy: delegate to the user function.
 * ------------------------------------------------------------------------
 */
static void lruDestroy(ts_algo_tree_t *tree,
                       lru_node_t    **n) {
	((ts_algo_lru_t*)tree->rsc)->onDestroy(tree,&(*n)->cont);
	free(*n); *n=NULL;
}

/* ------------------------------------------------------------------------
 * Allocate and initialise a new LRU object
 * ------------------------------------------------------------------------
 */
ts_algo_lru_t *ts_algo_lru_new(uint32_t max,
                               ts_algo_comprsc_t compare,
                               ts_algo_update_t  onUpdate,
                               ts_algo_delete_t  onDelete,
                               ts_algo_delete_t  onDestroy) {
	ts_algo_lru_t *lru;
	lru = malloc(sizeof(ts_algo_lru_t));
	if (lru == NULL) {
		return NULL;
	}
	if (ts_algo_lru_init(lru,max,
	                     compare,
	                    onUpdate,
	                    onDelete,
	                   onDestroy) != TS_ALGO_OK) {
		free(lru); return NULL;
	}
	return lru;
}

/* ------------------------------------------------------------------------
 * Initialise an already allocated LRU object
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_lru_init(ts_algo_lru_t *lru,
                              uint32_t       max,
                              ts_algo_comprsc_t compare,
                              ts_algo_update_t  onUpdate,
                              ts_algo_delete_t  onDelete,
                              ts_algo_delete_t  onDestroy) {
	ts_algo_rc_t rc;

	lru->max       = max;
	lru->compare   = compare;
	lru->onUpdate  = onUpdate;
	lru->onDelete  = onDelete;
	lru->onDestroy = onDestroy;

	rc = ts_algo_tree_init(&lru->tree,
	    (ts_algo_comprsc_t)&lruCompare,NULL,
	     (ts_algo_update_t)&lruUpdate,
	     (ts_algo_delete_t)&lruDelete,
	     (ts_algo_delete_t)&lruDestroy);
	if (rc != TS_ALGO_OK) return rc;

	lru->tree.rsc = lru;

	ts_algo_list_init(&lru->list);
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Destroy LRU object
 * ------------------------------------------------------------------------
 */
void ts_algo_lru_destroy(ts_algo_lru_t *lru) {
	ts_algo_tree_destroy(&lru->tree);
	ts_algo_list_destroy(&lru->list);
}

/* ------------------------------------------------------------------------
 * Get node
 * ------------------------------------------------------------------------
 */
void *ts_algo_lru_get(ts_algo_lru_t *lru,
                      void         *cont)
{
	lru_node_t n, *r;

	n.cont = cont;
	r = ts_algo_tree_find(&lru->tree, &n);
	if (r == NULL) return NULL;

	ts_algo_list_promote(&lru->list, r->lnode);

	return r->cont;
}

/* ------------------------------------------------------------------------
 * Promote resident
 * ------------------------------------------------------------------------
 */
static inline char promoteResident(ts_algo_lru_t *lru) {
	lru_node_t *l;
	ts_algo_list_node_t *tmp;

	tmp = lru->list.last;
	if (tmp == NULL) return 1;
	l = tmp->cont;

	if (!l->rsdnt) return 1;
	if (tmp == lru->list.head) return 1;

	ts_algo_list_remove(&lru->list, tmp);
	tmp->prv = NULL;
	lru->list.head->prv = tmp;
	tmp->nxt = lru->list.head;
	lru->list.head = tmp;
	lru->list.len++;

	return 0;
}

/* ------------------------------------------------------------------------
 * Promote residents
 * ------------------------------------------------------------------------
 */
static inline char promoteResidents(ts_algo_lru_t *lru) {
	for(int i=0; i<lru->list.len; i++) {
		if (lru->list.head == lru->list.last) return 0;
		if (promoteResident(lru)) return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------------
 * Remove (if necessary and possible)
 * ------------------------------------------------------------------------
 */
static inline void lruremove(ts_algo_lru_t *lru) {
	lru_node_t n;
	if (lru->max == 0 || lru->list.len < lru->max) return;
	if (lru->list.head == NULL) return;
	if (!promoteResidents(lru)) return;
	ts_algo_list_node_t *ln = lru->list.last;
	ts_algo_list_remove(&lru->list, ln);
	n.cont = ((lru_node_t*)ln->cont)->cont; free(ln);
	ts_algo_tree_delete(&lru->tree, &n);
}

/* ------------------------------------------------------------------------
 * Add node (either resident or not)
 * ------------------------------------------------------------------------
 */
static inline ts_algo_rc_t add2lru(ts_algo_lru_t *lru,
                                   void         *cont,
                                   char         rsdnt) {
	lru_node_t   *n;
	ts_algo_rc_t rc;

	n = malloc(sizeof(lru_node_t));
	if (n == NULL) return TS_ALGO_NO_MEM;

	n->cont = cont;
        n->rsdnt = rsdnt;
	rc = ts_algo_tree_insert(&lru->tree, n);
	if (rc != TS_ALGO_OK) {
		free(n); return rc;
	}
	rc = ts_algo_list_insert(&lru->list, n);
	if (rc != TS_ALGO_OK) {
		ts_algo_tree_delete(&lru->tree, n);
		return rc;
	}
	n->lnode = lru->list.head;
	lruremove(lru); return rc;
}

/* ------------------------------------------------------------------------
 * Add node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_lru_add(ts_algo_lru_t *lru,
                             void         *cont) {
	return add2lru(lru, cont, 0);
}

/* ------------------------------------------------------------------------
 * Add a value to the cache as resident.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_lru_addResident(ts_algo_lru_t *lru,
                                     void         *cont) {
	return add2lru(lru, cont, 1);
}

/* ------------------------------------------------------------------------
 * Revoke residence from a cache element.
 * ------------------------------------------------------------------------
 */
void ts_algo_lru_revokeResidence(ts_algo_lru_t *lru,
                                 void         *cont) {
	lru_node_t n, *r;

	n.cont = cont;
	r = ts_algo_tree_find(&lru->tree, &n);
	if (r == NULL) return;

	r->rsdnt = 0;
}
