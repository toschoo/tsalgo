
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tsalgo/map.h>

#define slot_t ts_algo_map_slot_t

#define MAKESLOT(k,s,d) \
	slot_t *slot = malloc(sizeof(slot_t)); \
	if (slot == NULL) return TS_ALGO_NO_MEM; \
	slot->key = malloc(s); \
	if (slot->key == NULL) {\
		free(slot); return TS_ALGO_NO_MEM; \
	}\
	memcpy(slot->key, k, s);\
	slot->ksz = s; \
	slot->data = d;

#define SLOT(x) ((slot_t*)x)

uint64_t ts_algo_hash_id(char *key, size_t ksz) {
	return (uint64_t)*key;
}

static ts_algo_rc_t bufinit(ts_algo_map_t *map) {
	map->buf = calloc(map->baseSize, sizeof(ts_algo_list_t));
	if (map->buf == NULL) return TS_ALGO_NO_MEM;
	for(int i=0; i<map->baseSize; i++) {
		ts_algo_list_init(map->buf+i);
	}
	return TS_ALGO_OK;
}

static inline ts_algo_rc_t copyall(ts_algo_map_t *src,
                                   ts_algo_map_t *trg)
{
	ts_algo_rc_t rc = TS_ALGO_OK;

	for(int i=0; i<src->curSize; i++) {
		for(ts_algo_list_node_t *run=src->buf[i].head; run!=NULL;) {
			ts_algo_map_slot_t *s = run->cont;
			if (s == NULL) break;
			uint64_t k = trg->hsh(s->key, s->ksz)%trg->curSize;
			rc = ts_algo_list_insert(&trg->buf[k], s);
			ts_algo_list_node_t *tmp = run->nxt;
			ts_algo_list_remove(src->buf+i, run);
			free(run);
			run = tmp;
		}
	}
	return rc;
}

static ts_algo_rc_t bufresize(ts_algo_map_t *map) {
	ts_algo_map_t tmp;
	ts_algo_rc_t rc = TS_ALGO_OK;

	rc = ts_algo_map_init(&tmp, map->curSize<<2, map->hsh, map->del);
	if (rc != TS_ALGO_OK) return rc;

	rc = copyall(map, &tmp);
	if (rc != TS_ALGO_OK) {
		ts_algo_map_destroy(&tmp);
		return rc;
	}

	ts_algo_list_t *buf = map->buf;
	map->buf = tmp.buf; tmp.buf = buf;
	tmp.curSize = map->curSize;
	map->curSize=tmp.baseSize;

	ts_algo_map_destroy(&tmp);

	return TS_ALGO_OK;
}

ts_algo_map_t *ts_algo_map_new(uint32_t sz,
                               ts_algo_hash_t   hsh,
                               ts_algo_delete_t del) {
	ts_algo_map_t *map = malloc(sizeof(ts_algo_map_t));
	if (map == NULL) return NULL;
	ts_algo_rc_t rc = ts_algo_map_init(map, sz, hsh, del);
	if (rc != TS_ALGO_OK) {
		free(map); return NULL;
	}
	return map;
}

/* currently not needed
static inline uint32_t popcount(uint32_t n) {
	uint32_t count = 0;
	while (n) { // improve!
 		count += n & 1;
		n >>= 1;
	}
	return count;
}
*/

ts_algo_rc_t ts_algo_map_init(ts_algo_map_t *map, uint32_t sz,
		                         ts_algo_hash_t   hsh,
		                         ts_algo_delete_t del) {
	if (map == NULL) return TS_ALGO_INVALID;
	map->baseSize = sz==0?8192:sz;
	map->curSize  = map->baseSize;
	map->count    = 0;
	map->factor   = 4;
	map->del      = del;
	map->hsh      = hsh;
	map->buf      = NULL;
	return bufinit(map);
}

static inline void listdestroy(ts_algo_list_t *list, ts_algo_delete_t del) {
	for(ts_algo_list_node_t *run=list->head; run!=NULL; run=run->nxt) {
		if (run->cont != NULL) {
			void *x = SLOT(run->cont)->data;
			free(SLOT(run->cont)->key);
			free(run->cont);
			if (del != NULL) {
				del(NULL, &x);
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

ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, char *key, size_t ksz, void *data) {
	ts_algo_rc_t rc=TS_ALGO_OK;
	if (map->count >= map->curSize<<1) {
		rc = bufresize(map);
	        if (rc != TS_ALGO_OK) return rc;
	}
	uint64_t k = map->hsh(key, ksz)%map->curSize;
	MAKESLOT(key, ksz, data);
	rc = ts_algo_list_insert(&map->buf[k], slot);
	map->count++;
	return rc;
}

static inline ts_algo_list_node_t *getnode(ts_algo_map_t *map, char *key, size_t ksz) {
	uint64_t k = map->hsh(key, ksz)%map->curSize;
	for(ts_algo_list_node_t *run=map->buf[k].head; run!=NULL; run=run->nxt) {
		if (memcmp(key, SLOT(run->cont)->key, ksz) == 0) return run;
	}
	return NULL;

}

static inline slot_t *getslot(ts_algo_map_t *map, char *key, size_t ksz) {
	ts_algo_list_node_t *n = getnode(map, key, ksz);
	if (n != NULL) return n->cont;
	return NULL;

}

void *ts_algo_map_get(ts_algo_map_t *map, char *key, size_t ksz) {
	slot_t *s = getslot(map, key, ksz);
	if (s != NULL) return s->data;
	return NULL;
}

void *ts_algo_map_remove(ts_algo_map_t *map, char *key, size_t ksz) {
	uint64_t k = map->hsh(key, ksz)%map->curSize;
	for(ts_algo_list_node_t *run=map->buf[k].head; run!=NULL; run=run->nxt) {
		if (memcmp(key, SLOT(run->cont)->key, ksz) == 0) {
			ts_algo_list_remove(map->buf+k, run);
			slot_t *slot = run->cont;
			void *data = slot->data;
			free(run); free(slot->key); free(slot);
			map->count--;
			return data;
		}
	}
	return NULL;
}

void ts_algo_map_delete(ts_algo_map_t *map, char *key, size_t ksz) {
	void *data = ts_algo_map_remove(map, key, ksz);
	if (data != NULL && map->del != NULL) map->del(NULL, data);
}

void *ts_algo_map_update(ts_algo_map_t *map, char *key, size_t ksz, void *data) {
	slot_t *s = getslot(map, key, ksz);
	if (s == NULL) return NULL;
	s->data = data;
	return data;
}

ts_algo_map_it_t *ts_algo_map_iterate(ts_algo_map_t *map) {
	ts_algo_map_it_t *it = calloc(1, sizeof(ts_algo_map_it_t));
	if (it == NULL) return NULL;
	it->map = map;
	ts_algo_map_it_reset(it);
	return it;
}

char ts_algo_map_it_eof(ts_algo_map_it_t *it) {
	if (it->count > it->map->count) return 1;
	return 0;
}

void ts_algo_map_it_reset(ts_algo_map_it_t *it) {
	it->count = 0;
	it->slot  = 0;
	it->entry = -1;
	ts_algo_map_it_advance(it);
}

void ts_algo_map_it_advance(ts_algo_map_it_t *it) {
	/*
	if (it->count > it->map->count) {
		fprintf(stderr, "return on slot %u\n", it->slot);
		// it->slot = it->map->curSize;
		return;
	}
	*/
	if (it->slot >= it->map->curSize) {
		// fprintf(stderr, "return on slot %u\n", it->slot);
		return;
	}
	// fprintf(stderr, "advance: %d | %d\n", it->entry, it->map->buf[it->slot].len);
	if (it->entry+1 == it->map->buf[it->slot].len) {
		it->entry = 0;
		do {
			it->slot++;
		} while(it->slot < it->map->curSize &&
		        it->map->buf[it->slot].len == 0);
	} else {
		it->entry++;
	}
	it->count++;
}

ts_algo_map_slot_t *ts_algo_map_it_get(ts_algo_map_it_t *it) {
	// fprintf(stderr, "COUNT: %u > %u\n", it->count, it->map->count);
	// if (it->count > it->map->count) return NULL; // :-(
	if (it->slot >= it->map->curSize) return NULL;
	int i=0;
	for(ts_algo_list_node_t *run=it->map->buf[it->slot].head;
	    run!=NULL; run=run->nxt)
	{
		if (i == it->entry) return run->cont;
		i++;
	}
	return NULL;
}

void ts_algo_map_showslots(ts_algo_map_t *map) {
	for (int i=0;i<map->curSize;i++) {
		fprintf(stderr, "%06d: %d\n", i, map->buf[i].len);
	}
}
