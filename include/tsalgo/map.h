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

typedef uint64_t (*ts_algo_hash_t)(char*,size_t);

uint64_t ts_algo_hash_id(char* key, size_t ksz);

typedef struct {
  uint32_t    baseSize; 
  uint32_t     curSize; 
  uint32_t      factor; 
  uint32_t       count;
  ts_algo_hash_t   hsh;
  ts_algo_delete_t del;
  ts_algo_list_t  *buf; 
} ts_algo_map_t;

typedef struct {
  ts_algo_map_t *map;
  int           slot;
  int          entry;
  uint32_t     count;
} ts_algo_map_it_t;

typedef struct {
	char  *key;
	size_t ksz;
	void *data;
} ts_algo_map_slot_t;

ts_algo_map_t *ts_algo_map_new(uint32_t sz,
                      ts_algo_hash_t   hsh,
                      ts_algo_delete_t del);
ts_algo_rc_t ts_algo_map_init(ts_algo_map_t *map, uint32_t sz,
                                         ts_algo_hash_t   hsh,
                                         ts_algo_delete_t del);
void ts_algo_map_destroy(ts_algo_map_t *map);

ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, char *key, size_t ksz, void *data);
void *ts_algo_map_get(ts_algo_map_t *map, char *key, size_t ksz);
void *ts_algo_map_remove(ts_algo_map_t *map, char *key, size_t ksz);
void ts_algo_map_delete(ts_algo_map_t *map, char *key, size_t ksz);
void *ts_algo_map_update(ts_algo_map_t *map, char *key, size_t ksz, void *data);

ts_algo_map_it_t *ts_algo_map_iterate(ts_algo_map_t *map);
void ts_algo_map_it_advance(ts_algo_map_it_t *it);
char ts_algo_map_it_eof(ts_algo_map_it_t *it);
void ts_algo_map_it_reset(ts_algo_map_it_t *it);
ts_algo_map_slot_t *ts_algo_map_it_get(ts_algo_map_it_t *it);

void ts_algo_map_showslots(ts_algo_map_t *map);

#endif
