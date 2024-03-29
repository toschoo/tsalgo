/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * The List Datatype
 * ========================================================================
 */
#include <tsalgo/list.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------------
 * init
 * ------------------------------------------------------------------------
 */
void ts_algo_list_init(ts_algo_list_t *list) {
	list->head=NULL;
	list->last=NULL;
	list->len =0;
}

/* ------------------------------------------------------------------------
 * destroy
 * ------------------------------------------------------------------------
 */
static void destroy(ts_algo_list_node_t *node) {
	while (node->nxt != NULL) {
		ts_algo_list_node_t *tmp = node->nxt->nxt;
		free(node->nxt); node->nxt = tmp;
	}
}

void ts_algo_list_destroy(ts_algo_list_t *list) {
	if (list->head != NULL) {
		destroy(list->head); free(list->head);
	}
	ts_algo_list_init(list);
}
void ts_algo_list_destroyAll(ts_algo_list_t *list) {
	ts_algo_list_node_t *runner=list->head;
	while(runner!=NULL) {
		if (runner->cont!=NULL) {
			free(runner->cont);
			runner->cont=NULL;
		}
		runner=runner->nxt;
	}
	ts_algo_list_destroy(list);
}

static inline ts_algo_rc_t insert(ts_algo_list_t *list, void *cont,
                                  ts_algo_list_node_t        *node) {
	ts_algo_list_node_t *tmp;

	if (list->head == NULL) {
		list->head = node;
		if (list->head == NULL) return -1;
		list->head->nxt=NULL;
		list->head->prv=NULL;
		list->head->cont=cont;
		list->last=list->head;
		list->len=1;
		return 0;
	}
	tmp=list->head;
	list->head=node;
	if (list->head == NULL) return -1;
	list->head->nxt=tmp;
	tmp->prv=list->head;
	list->head->prv=NULL;
	list->head->cont=cont;
	list->len++;
	return 0;
}

/* ------------------------------------------------------------------------
 * insert allocating new list node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_insert(ts_algo_list_t *list, void *cont) {
	ts_algo_list_node_t *node = malloc(sizeof(ts_algo_list_node_t));
	if (node == NULL) return TS_ALGO_NO_MEM;
	ts_algo_rc_t rc = insert(list, cont, node);
	if (rc != TS_ALGO_OK) free(node);
	return rc;
}

/* ------------------------------------------------------------------------
 * insert reusing list node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_insertNode(ts_algo_list_t *list, void *cont,
                                     ts_algo_list_node_t        *node) {
	return insert(list, cont, node);
}

/* ------------------------------------------------------------------------
 * Helper: append
 * ------------------------------------------------------------------------
 */
static inline ts_algo_rc_t append(ts_algo_list_t *list, void *cont,
                                  ts_algo_list_node_t        *node) {
	if (list->last == NULL) return insert(list,cont,node);
	list->last->nxt = node;
	if (list->last->nxt == NULL) return -1;
	list->last->nxt->nxt=NULL;
	list->last->nxt->prv=list->last;
	list->last->nxt->cont=cont;
	list->last=list->last->nxt;
	list->len++;
	return 0;
}

/* ------------------------------------------------------------------------
 * append allocating new list node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_append(ts_algo_list_t *list, void *cont) {
	ts_algo_list_node_t *node = malloc(sizeof(ts_algo_list_node_t));
	if (node == NULL) return TS_ALGO_NO_MEM;
	ts_algo_rc_t rc = append(list, cont, node);
	if (rc != TS_ALGO_OK) free(node);
	return rc;
}

/* ------------------------------------------------------------------------
 * append reusing list node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_appendNode(ts_algo_list_t *list, void *cont,
                                     ts_algo_list_node_t        *node) {
	return append(list, cont, node);
}


/* ------------------------------------------------------------------------
 * remove
 * ------------------------------------------------------------------------
 */
void ts_algo_list_remove(ts_algo_list_t *list, ts_algo_list_node_t *node)
{
	int existed = 0;
	if (node == NULL) return;
	if (node == list->head) {
		list->head=node->nxt;
		if (list->head != NULL) list->head->prv=NULL;
		if (node != list->last) {
			list->len--;
			return;
		}
	}
	if (node == list->last) {
		list->last=list->last->prv;
		if (list->last != NULL) list->last->nxt=NULL;
		list->len--;
		return;
	}
	if (node->prv != NULL) {
		node->prv->nxt = node->nxt;
		existed = 1;
	}
	if (node->nxt != NULL) {
		node->nxt->prv = node->prv;
		existed = 1;
	}
	if (existed) list->len--;
}

/* ------------------------------------------------------------------------
 * promote
 * ------------------------------------------------------------------------
 */
void ts_algo_list_promote(ts_algo_list_t *list, ts_algo_list_node_t *node) {

	if (node == list->head) return;

	ts_algo_list_node_t *tmp = node->prv;
	ts_algo_list_remove(list, node);
	node->nxt = tmp;
	node->prv = tmp->prv;
	if (tmp->prv != NULL) tmp->prv->nxt = node;
	tmp->prv = node;
	if (tmp == list->head) list->head = node;
	list->len++;
}

/* ------------------------------------------------------------------------
 * degrade
 * ------------------------------------------------------------------------
 */
void ts_algo_list_degrade(ts_algo_list_t *list, ts_algo_list_node_t *node) {

	if (node == list->last) return;

	ts_algo_list_node_t *tmp = node->nxt;
	ts_algo_list_remove(list, node);
	node->prv = tmp;
	node->nxt = tmp->nxt;
	if (tmp->nxt != NULL) tmp->nxt->prv = node;
	tmp->nxt = node;
	if (tmp == list->last) list->last = node;
	list->len++;
}

/* ------------------------------------------------------------------------
 * copy
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_copy(ts_algo_list_t *list) {
	ts_algo_list_t *c;
	ts_algo_list_node_t *runner;

	c = malloc(sizeof(ts_algo_list_t));
	if (c == NULL) return NULL;
	ts_algo_list_init(c);

	runner=list->head;
	while (runner != NULL) {
		if (ts_algo_list_append(c,runner->cont) != TS_ALGO_OK) 
		{
			ts_algo_list_destroy(c); free(c);
			return NULL;
		}
		runner = runner->nxt;
	}
	return c;	
}

/* ------------------------------------------------------------------------
 * reverse
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_reverse(ts_algo_list_t *list) {
	ts_algo_list_t *r;
	ts_algo_list_node_t *runner;

	if (list->len == 0) return NULL;

	r = malloc(sizeof(ts_algo_list_t));
	if (r == NULL) return NULL;

	ts_algo_list_init(r);

	runner = list->head;
	while (runner != NULL) {
		if (ts_algo_list_insert(r,runner->cont) != TS_ALGO_OK) 
		{
			ts_algo_list_destroy(r); free(r);
			return NULL;
		}
		runner = runner->nxt;
	}
	return r;
}

