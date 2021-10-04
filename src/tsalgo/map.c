
#include <stdlib.h>
#include <string.h>
#include <tsalgo/map.h>

static ts_algo_rc_t bufinit(ts_algo_map_t *map) {
	map->buf = calloc(map->baseSize, sizeof(ts_algo_list_t));
	if (map->buf == NULL) return TS_ALGO_NO_MEM;
	for(int i=0; i<map->baseSize; i++) {
		ts_algo_list_init(map->buf+i);
	}
	return TS_ALGO_OK;
}

static ts_algo_rc_t bufresize(ts_algo_map_t *map) {
	return TS_ALGO_INVALID;
}

ts_algo_map_t *ts_algo_map_new(uint32_t sz,
		               ts_algo_grab_t  grab,
                               ts_algo_delete_t del) {
	ts_algo_map_t *map = malloc(sizeof(ts_algo_map_t));
	if (map == NULL) return NULL;
	ts_algo_rc_t rc = ts_algo_map_init(map, sz, grab, del);
	if (rc != TS_ALGO_OK) {
		free(map); return NULL;
	}
	return map;
}

ts_algo_rc_t ts_algo_map_init(ts_algo_map_t *map, uint32_t sz,
                                          ts_algo_grab_t grab,
		                         ts_algo_delete_t del) {
	if (map == NULL) return TS_ALGO_INVALID;
	map->baseSize = sz==0?8192:sz;
	map->curSize  = map->baseSize;
	map->count    = 0;
	map->factor   = 4;
	map->grab     = grab;
	map->del      = del;
	map->buf      = NULL;
	return bufinit(map);
}

static void listdestroy(ts_algo_list_t *list, ts_algo_delete_t del) {
	if (del != NULL) {
		for(ts_algo_list_node_t *run=list->head; run!=NULL; run=run->nxt) {
			if (run->cont != NULL) {
				del(NULL, run->cont);
			}
		}
	}
}

void ts_algo_map_destroy(ts_algo_map_t *map) {
	if (map == NULL) return;
	if (map->buf == NULL) return;
	for(int i=0;i<map->curSize;i++) {
		listdestroy(map->buf+i, map->del);
		ts_algo_list_destroy(map->buf+i);
	}
	free(map->buf); map->buf = NULL;
}

ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, void *data) {
	ts_algo_rc_t rc=TS_ALGO_OK;
	if (map->count > map->curSize<<2) {
		rc = bufresize(map);
	        if (rc != TS_ALGO_OK) return rc;
	}
	uint64_t k = map->grab(data);
	uint64_t h = k%map->curSize;
	rc = ts_algo_list_insert(&map->buf[h], data);
	return rc;	
}

void *ts_algo_map_get(ts_algo_map_t *map, uint64_t key) {
	uint64_t k = key%map->curSize;
	for(ts_algo_list_node_t *run=map->buf[k].head; run!=NULL; run=run->nxt) {
		if (key == map->grab(run->cont)) return run->cont;
	}
	return NULL;
}

void *ts_algo_map_remove(ts_algo_map_t *map, uint64_t key) {
	uint64_t k = key%map->curSize;
	for(ts_algo_list_node_t *run=map->buf[k].head; run!=NULL; run=run->nxt) {
		if (key == map->grab(run->cont)) {
			ts_algo_list_remove(map->buf+k, run);
			void *data = run->cont;
			free(run);
			return data;
		}
	}
	return NULL;
}

void ts_algo_map_delete(ts_algo_map_t *map, uint64_t key) {
	void *data = ts_algo_map_remove(map, key);
	if (data != NULL) map->del(NULL, data);
}

ts_algo_rc_t ts_algo_map_update(ts_algo_map_t *map, uint64_t key);
