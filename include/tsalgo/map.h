/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2021
 * ========================================================================
 * The Map Datatype
 * ========================================================================
 */
#ifndef ts_algo_map_decl
#define ts_algo_map_decl

#include <tsalgo/types.h>
#include <tsalgo/list.h>

typedef struct {
  uint32_t    baseSize; 
  uint32_t     curSize; 
  uint32_t      factor; 
  uint32_t       count;
  ts_algo_delete_t del;
  ts_algo_list_t  *buf; 
} ts_algo_map_t;

ts_algo_map_t *ts_algo_map_new(uint32_t sz,
                      ts_algo_delete_t del);
ts_algo_rc_t ts_algo_map_init(ts_algo_map_t *map, uint32_t sz,
                                         ts_algo_delete_t del);
void ts_algo_map_destroy(ts_algo_map_t *map);
ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, uint64_t key, void *data);
void *ts_algo_map_get(ts_algo_map_t *map, uint64_t key);
void *ts_algo_map_remove(ts_algo_map_t *map, uint64_t key);
void ts_algo_map_delete(ts_algo_map_t *map, uint64_t key);
ts_algo_rc_t ts_algo_map_update(ts_algo_map_t *map, uint64_t key);
void ts_algo_map_showslots(ts_algo_map_t *map);

#endif
