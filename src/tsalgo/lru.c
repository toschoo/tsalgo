/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 * LRU Cache
 * ========================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <tsalgo/lru.h>

typedef struct {
	void *cont;
	ts_algo_list_node_t *lnode;
} lru_node_t;

static ts_algo_cmp_t lruCompare(ts_algo_tree_t *tree,
                                lru_node_t     *one,
                                lru_node_t     *two) {
	return ((ts_algo_lru_t*)tree->rsc)->compare(
	                 NULL, one->cont, two->cont);
}

static ts_algo_rc_t lruUpdate(ts_algo_tree_t *tree,
                              lru_node_t     *oldN,
                              lru_node_t     *newN) {
	ts_algo_rc_t rc;
	rc = ((ts_algo_lru_t*)tree->rsc)->onUpdate(tree,oldN->cont,
	                                                newN->cont);
	if (rc == TS_ALGO_OK) free(newN);
	return rc;
}

static void lruDelete(ts_algo_tree_t *tree,
                      lru_node_t    **n) {
	((ts_algo_lru_t*)tree->rsc)->onDelete(tree,&(*n)->cont);
	free(*n); *n=NULL;
}

static void lruDestroy(ts_algo_tree_t *tree,
                       lru_node_t    **n) {
	((ts_algo_lru_t*)tree->rsc)->onDestroy(tree,&(*n)->cont);
	free(*n); *n=NULL;
}

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

void ts_algo_lru_destroy(ts_algo_lru_t *lru) {
	ts_algo_tree_destroy(&lru->tree);
	ts_algo_list_destroy(&lru->list);
}

void *ts_algo_lru_get(ts_algo_lru_t *lru,
                      void         *cont)
{
	lru_node_t n, *r;
	n.cont = cont;
	r = ts_algo_tree_find(&lru->tree, &n);
	if (r == NULL) return NULL;

	/* make this a list service! */
	ts_algo_list_remove(&lru->list, r->lnode);
	r->lnode->prv = NULL;
	r->lnode->nxt = lru->list.head;
	if (lru->list.head != NULL) lru->list.head->prv = r->lnode;
	lru->list.head = r->lnode;
	if (lru->list.last == NULL) lru->list.last = r->lnode;
	lru->list.len++;

	return r->cont;
}

static inline void lruremove(ts_algo_lru_t *lru) {
	if (lru->max == 0 || lru->list.len < lru->max) return;
	lru_node_t n;
	ts_algo_list_node_t *ln = lru->list.last;
	if (ln == NULL) {
		fprintf(stderr, "NO LAST\n"); return;
	}
	ts_algo_list_remove(&lru->list, ln);
	n.cont = ln->cont; free(ln);
	ts_algo_tree_delete(&lru->tree, &n);
}

ts_algo_rc_t ts_algo_lru_add(ts_algo_lru_t *lru,
                             void         *cont) {
	lru_node_t   *n;
	ts_algo_rc_t rc;
	n = malloc(sizeof(lru_node_t));
	if (n == NULL) return TS_ALGO_NO_MEM;

	n->cont = cont;
	rc = ts_algo_tree_insert(&lru->tree, n);
	if (rc != TS_ALGO_OK) {
		free(n); return rc;
	}
	rc = ts_algo_list_insert(&lru->list, cont);
	if (rc != TS_ALGO_OK) {
		ts_algo_tree_delete(&lru->tree, n);
		return rc;
	}
	n->lnode = lru->list.head;
	lruremove(lru); return rc;
}
