/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 * LRU Cache
 * ---------
 * Provides the basic functionality for an LRU Cache.
 * Note that this service provides only the basis for implementing
 * real caches. In particular,
 * - get/add is not atomic; these are two distinct calls
 * - the services are *not* thread safe;
 *   a real implementation still needs to protect the cache
 *   to allow concurrent access.
 * ========================================================================
 */
#ifndef ts_algo_lru_decl
#define ts_algo_lru_decl

#include <tsalgo/types.h>
#include <tsalgo/tree.h>
#include <tsalgo/list.h>

/* ------------------------------------------------------------------------
 * An LRU Cache that grows infinitely
 * (is not really an LRU Cache...)
 * ------------------------------------------------------------------------
 */
#define TS_ALGO_LRU_INF 0

/* ------------------------------------------------------------------------
 * LRU Cache
 * ------------------------------------------------------------------------
 */
typedef struct {
	ts_algo_tree_t    tree;      /* to find nodes quickly        */
	ts_algo_list_t    list;      /* the last has to go if needed */
	uint32_t          max;       /* max size of the cache        */
	ts_algo_comprsc_t compare;   /* user compare callback        */
	ts_algo_update_t  onUpdate;  /* user update  callback        */
	ts_algo_delete_t  onDelete;  /* user delete  callback        */
	ts_algo_delete_t  onDestroy; /* user destroy callback        */
} ts_algo_lru_t;

/* ------------------------------------------------------------------------
 * Allocate and initialise a new LRU Cache
 * ------------------------------------------------------------------------
 */
ts_algo_lru_t *ts_algo_lru_new(uint32_t          max,
                               ts_algo_comprsc_t compare,
                               ts_algo_update_t  onUpdate,
                               ts_algo_delete_t  onDelete,
                               ts_algo_delete_t  onDestroy);

/* ------------------------------------------------------------------------
 * Initialise an already allocated LRU Cache
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_lru_init(ts_algo_lru_t *lru,
                              uint32_t       max,
                              ts_algo_comprsc_t compare,
                              ts_algo_update_t  onUpdate,
                              ts_algo_delete_t  onDelete,
                              ts_algo_delete_t  onDestroy);

/* ------------------------------------------------------------------------
 * Destroy an LRU Cache
 * NOTE: if the LRU Cache was allocated dynamically,
 *       the memory pointed to by 'lru' still must be freed.
 * ------------------------------------------------------------------------
 */
void ts_algo_lru_destroy(ts_algo_lru_t *lru);

/* ------------------------------------------------------------------------
 * Get a value from the cache.
 * If the value is not in the cache, NULL is returned.
 * ------------------------------------------------------------------------
 */
void *ts_algo_lru_get(ts_algo_lru_t *lru,
                      void         *cont);

/* ------------------------------------------------------------------------
 * Add a value to the cache.
 * The function may fail, when not enough memory is available.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_lru_add(ts_algo_lru_t *lru,
                             void         *cont);
#endif
