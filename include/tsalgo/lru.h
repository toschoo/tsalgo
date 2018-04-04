/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 * LRU Cache
 * ========================================================================
 */
#ifndef ts_algo_lru_decl
#define ts_algo_lru_decl

#include <tsalgo/types.h>
#include <tsalgo/tree.h>
#include <tsalgo/list.h>

typedef struct {
	ts_algo_tree_t tree;
	ts_algo_list_t list;
	uint32_t        max;
	ts_algo_comprsc_t compare;
	ts_algo_update_t onUpdate;
	ts_algo_delete_t onDelete;
	ts_algo_delete_t onDestroy;
} ts_algo_lru_t;

ts_algo_lru_t *ts_algo_lru_new(uint32_t          max,
                               ts_algo_comprsc_t compare,
                               ts_algo_update_t  onUpdate,
                               ts_algo_delete_t  onDelete,
                               ts_algo_delete_t  onDestroy);

ts_algo_rc_t ts_algo_lru_init(ts_algo_lru_t *lru,
                              uint32_t       max,
                              ts_algo_comprsc_t compare,
                              ts_algo_update_t  onUpdate,
                              ts_algo_delete_t  onDelete,
                              ts_algo_delete_t  onDestroy);

void ts_algo_lru_destroy(ts_algo_lru_t *lru);

void *ts_algo_lru_get(ts_algo_lru_t *lru,
                      void         *cont);

ts_algo_rc_t ts_algo_lru_add(ts_algo_lru_t *lru,
                             void *cont);


#endif
