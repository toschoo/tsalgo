/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * The List Datatype
 * ========================================================================
 */
#ifndef ts_algo_list_decl
#define ts_algo_list_decl

#include <tsalgo/types.h>

/* ------------------------------------------------------------------------
 * List node
 * ------------------------------------------------------------------------
 */
typedef struct ts_algo_list_node_st {
	struct ts_algo_list_node_st *nxt;
	struct ts_algo_list_node_st *prv;
	void  *cont;
} ts_algo_list_node_t;

/* ------------------------------------------------------------------------
 * The list datatype
 * ------------------------------------------------------------------------
 */
typedef struct {
	ts_algo_list_node_t *head;
	ts_algo_list_node_t *last;
	uint32_t             len;
} ts_algo_list_t;

/* ------------------------------------------------------------------------
 * init
 * ------------------------------------------------------------------------
 */
void ts_algo_list_init(ts_algo_list_t *list); 

/* ------------------------------------------------------------------------
 * destroy
 * -------
 * destroys only the list structure, but leaves the content untouched
 * running time: O(n)
 * ------------------------------------------------------------------------
 */
void ts_algo_list_destroy(ts_algo_list_t *list); 

/* ------------------------------------------------------------------------
 * destroyAll
 * ----------
 * destroys the list structure and the content
 * running time: O(n)
 * ------------------------------------------------------------------------
 */
void ts_algo_list_destroyAll(ts_algo_list_t *list); 

/* ------------------------------------------------------------------------
 * insert 
 * ------
 * Inserts an element at the head of the list.
 * running time: O(1)
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_insert(ts_algo_list_t *list, void *cont);

/* ------------------------------------------------------------------------
 * append
 * ------
 * Adds an element at the end of the list.
 * running time: O(1)
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_list_append(ts_algo_list_t *list, void *cont);

/* ------------------------------------------------------------------------
 * remove
 * ------
 * Removes the indicated note from the list, 
 * but does not release its memory.
 * running time: O(1)
 * ------------------------------------------------------------------------
 */
void ts_algo_list_remove(ts_algo_list_t *list, ts_algo_list_node_t *node);

/* ------------------------------------------------------------------------
 * copy
 * ----
 * 
 * copy allocates a new list and copies the structure
 * of the original list, but not of the content.
 * The content of the resulting list points to the same memory location
 * as the content of the original list.
 * running time: O(n)
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_copy(ts_algo_list_t *list);

/* ------------------------------------------------------------------------
 * reverse
 * -------
 * 
 * reverses the input list.
 * reverse allocates a new list and copies the structure
 * of the original list, but not the content.
 * The content of the resulting list points to the same memory location
 * as the content of the original list.
 * running time: O(n)
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_reverse(ts_algo_list_t *list);

/* ------------------------------------------------------------------------
 * sort
 * ----
 *
 * sorts the input list.
 * sort allocates a new list and copies the structure
 * of the original list, but not the content.
 * The content of the resulting list points to the same memory location
 * as the content of the original list.
 * running time: O(log(n) * n)
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_sort(ts_algo_list_t   *list,
                                  ts_algo_compare_t compare);

/* ------------------------------------------------------------------------
 * merge
 * -----
 * 
 * merges two (sorted) lists into a new (sorted) list.
 * merge allocates a new list and copies the structure
 * of the original list, but not the content.
 * The content of the resulting list points to the same memory location
 * as the content of the original list.
 * running time: O(n+m)
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_merge(ts_algo_list_t   *l1,
                                   ts_algo_list_t   *l2,
                                   ts_algo_compare_t compare);
#endif
