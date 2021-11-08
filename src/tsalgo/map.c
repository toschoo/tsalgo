/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2021
 * ========================================================================
 * The Map Datatype Implementation
 * ========================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tsalgo/map.h>

/* ----------------------------------------------------------------------------
 * Just a shorthand
 * ----------------------------------------------------------------------------
 */
#define slot_t ts_algo_map_slot_t

/* ----------------------------------------------------------------------------
 * Create a new slot
 * ----------------------------------------------------------------------------
 */
#define MAKESLOT(k,s,h,d) \
	slot_t *slot = malloc(sizeof(slot_t)); \
	if (slot == NULL) return TS_ALGO_NO_MEM; \
	slot->key = malloc(s); \
	if (slot->key == NULL) {\
		free(slot); return TS_ALGO_NO_MEM; \
	}\
	memcpy(slot->key, k, s);\
	slot->ksz = s; \
	slot->data = d; \
	slot->hash = h;

/* ----------------------------------------------------------------------------
 * Another shorthand
 * ----------------------------------------------------------------------------
 */
#define SLOT(x) ((slot_t*)x)

/* ----------------------------------------------------------------------------
 * Implementation of the Id Hash
 * ----------------------------------------------------------------------------
 */
uint64_t ts_algo_hash_id(const char *key, size_t ksz) {
	if (key == NULL) return 0;
	return (uint64_t)*key;
}

/* ----------------------------------------------------------------------------
 * Helper: initialise the buffer
 * ----------------------------------------------------------------------------
 */
static ts_algo_rc_t bufinit(ts_algo_map_t *map) {
	map->buf = calloc(map->baseSize, sizeof(ts_algo_list_t));
	if (map->buf == NULL) return TS_ALGO_NO_MEM;
	for(int i=0; i<map->baseSize; i++) {
		ts_algo_list_init(map->buf+i);
	}
	return TS_ALGO_OK;
}

/* ----------------------------------------------------------------------------
 * Helper: move all entries from src to trg,
 *         src is empty afterwards!
 * ----------------------------------------------------------------------------
 */
static inline ts_algo_rc_t moveall(ts_algo_map_t *src,
                                   ts_algo_map_t *trg)
{
	ts_algo_rc_t rc = TS_ALGO_OK;

	for(int i=0; i<src->curSize; i++) {
		for(ts_algo_list_node_t *run=src->buf[i].head; run!=NULL;) {
			ts_algo_map_slot_t *s = run->cont;
			if (s == NULL) break;
			uint64_t k = s->hash%trg->curSize;
			rc = ts_algo_list_insert(&trg->buf[k], s);
			ts_algo_list_node_t *tmp = run->nxt;
			ts_algo_list_remove(src->buf+i, run);
			free(run); // we could reuse the memory!
			run = tmp;
		}
	}
	trg->count = src->count;
	return rc;
}

/* ----------------------------------------------------------------------------
 * Helper: resize the buffer
 * ----------------------------------------------------------------------------
 */
static ts_algo_rc_t bufresize(ts_algo_map_t *map) {
	ts_algo_map_t tmp;
	ts_algo_rc_t rc = TS_ALGO_OK;

	// initialise a local map, 4 times as big as the original
	rc = ts_algo_map_init(&tmp, map->curSize<<2, map->hsh, map->del);
	if (rc != TS_ALGO_OK) return rc;

	// Move all entries from the original to the local map
	rc = moveall(map, &tmp);
	if (rc != TS_ALGO_OK) {
		ts_algo_map_destroy(&tmp);
		return rc;
	}

	// swap the buffers
	ts_algo_list_t *buf = map->buf;
	map->buf = tmp.buf; tmp.buf = buf;
	tmp.curSize = map->curSize;
	map->curSize=tmp.baseSize;

	// destroy the local map
	ts_algo_map_destroy(&tmp);

	return TS_ALGO_OK;
}

/* ----------------------------------------------------------------------------
 * Allocate a new map and initialise it
 * ----------------------------------------------------------------------------
 */
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

/* ----------------------------------------------------------------------------
 * Initialise an already allocated map
 * ----------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_map_init(ts_algo_map_t *map, uint32_t sz,
		                         ts_algo_hash_t   hsh,
		                         ts_algo_delete_t del) {
	if (map == NULL) return TS_ALGO_INVALID;
	map->baseSize = sz==0?8192:sz;
	map->curSize  = map->baseSize;
	map->count    = 0;
	map->del      = del;
	map->hsh      = hsh;
	map->buf      = NULL;
	return bufinit(map);
}

/* ----------------------------------------------------------------------------
 * Helper: Destroy everything in the list
 * ----------------------------------------------------------------------------
 */
static inline void listdestroy(ts_algo_list_t *list, ts_algo_delete_t del) {
	for(ts_algo_list_node_t *run=list->head; run!=NULL; run=run->nxt) {
		if (run->cont != NULL) {
			void *x = SLOT(run->cont)->data;
			free(SLOT(run->cont)->key);
			free(run->cont);
			if (del != NULL) { // we manage the memory
				del(NULL, &x);
			}
		}
	}
}

/* ----------------------------------------------------------------------------
 * Destroy an existing map
 * ----------------------------------------------------------------------------
 */
void ts_algo_map_destroy(ts_algo_map_t *map) {
	if (map == NULL) return;
	if (map->buf == NULL) return;
	for(int i=0;i<map->curSize;i++) {
		listdestroy(map->buf+i, map->del);
		ts_algo_list_destroy(map->buf+i);
	}
	free(map->buf); map->buf = NULL;
}

/* ----------------------------------------------------------------------------
 * Add a new element to the map
 * ----------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, char *key, size_t ksz, void *data) {
	ts_algo_rc_t rc=TS_ALGO_OK;
	// if we outgrew the map (twice as many elements as slots)
	// resize the buffer!
	if (map->count >= map->curSize<<1) {
		rc = bufresize(map);
	        if (rc != TS_ALGO_OK) return rc;
	}
	// compute the hash
	uint64_t k = map->hsh(key, ksz);
	// make a slot (this also remembers the hash,
	// so we don't need to compute it again when resizing;
	// error handling is defined in the macro
	MAKESLOT(key, ksz, k, data);
	// compute the modulus
	k%=map->curSize;
	// insert into the list
	rc = ts_algo_list_insert(&map->buf[k], slot);
	map->count++;
	if (rc != TS_ALGO_OK) {
		free(slot->key);
		free(slot);
	}
	return rc;
}

/* ----------------------------------------------------------------------------
 * Helper: Get the list node associated with a key
 * ----------------------------------------------------------------------------
 */
static inline ts_algo_list_node_t *getnode(ts_algo_map_t *map, char *key, size_t ksz) {
	uint64_t k = map->hsh(key, ksz)%map->curSize;
	for(ts_algo_list_node_t *run=map->buf[k].head; run!=NULL; run=run->nxt) {
		if (memcmp(key, SLOT(run->cont)->key, ksz) == 0) return run;
	}
	return NULL;

}

/* ----------------------------------------------------------------------------
 * Helper: Get the slot associated with a key
 * ----------------------------------------------------------------------------
 */
static inline slot_t *getslot(ts_algo_map_t *map, char *key, size_t ksz) {
	ts_algo_list_node_t *n = getnode(map, key, ksz);
	if (n != NULL) return n->cont;
	return NULL;

}

/* ----------------------------------------------------------------------------
 * Get the data associated with a key
 * ----------------------------------------------------------------------------
 */
void *ts_algo_map_get(ts_algo_map_t *map, char *key, size_t ksz) {
	slot_t *s = getslot(map, key, ksz);
	if (s != NULL) return s->data;
	return NULL;
}

/* ----------------------------------------------------------------------------
 * Remove a key from the map
 * ----------------------------------------------------------------------------
 */
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

/* ----------------------------------------------------------------------------
 * Delete a key from the map, i.e. remove it from the key and free the memory.
 * ----------------------------------------------------------------------------
 */
void ts_algo_map_delete(ts_algo_map_t *map, char *key, size_t ksz) {
	void *data = ts_algo_map_remove(map, key, ksz);
	if (data != NULL && map->del != NULL) map->del(NULL, data);
}

/* ----------------------------------------------------------------------------
 * Overwrite the data assocated with a key and return
 * - NULL if the key does not exist in the map
 * - The old data otherwise
 * ----------------------------------------------------------------------------
 */
void *ts_algo_map_update(ts_algo_map_t *map, char *key, size_t ksz, void *data) {
	slot_t *s = getslot(map, key, ksz);
	if (s == NULL) return NULL;
	void *d = s->data;
	s->data = data;
	return d;
}

/* ----------------------------------------------------------------------------
 * Create an iterator for the map
 * ----------------------------------------------------------------------------
 */
ts_algo_map_it_t *ts_algo_map_iterate(ts_algo_map_t *map) {
	ts_algo_map_it_t *it = calloc(1, sizeof(ts_algo_map_it_t));
	if (it == NULL) return NULL;
	it->map = map;
	ts_algo_map_it_reset(it);
	return it;
}

/* ----------------------------------------------------------------------------
 * Check if the iterator reached the end of the map.
 * We reached the iterator when we more often
 * than there are elements in the map.
 * ----------------------------------------------------------------------------
 */
char ts_algo_map_it_eof(ts_algo_map_it_t *it) {
	if (it->count > it->map->count) return 1;
	return 0;
}

/* ----------------------------------------------------------------------------
 * Rewind the iterator.
 * ----------------------------------------------------------------------------
 */
void ts_algo_map_it_reset(ts_algo_map_it_t *it) {
	it->count = 0;
	it->slot  = 0;
	it->entry = -1; // note that we start below the first element
	ts_algo_map_it_advance(it);
}

/* ----------------------------------------------------------------------------
 * Advance the iterator
 * ----------------------------------------------------------------------------
 */
void ts_algo_map_it_advance(ts_algo_map_it_t *it) {
	// we passed beyond the last slot
	// note: if it->count <= map.count,
	//       something is wrong!
	if (it->slot >= it->map->curSize) return;
	// advance slot
	if (it->entry+1 == it->map->buf[it->slot].len) {
		it->entry = 0;
		do {
			it->slot++;
		} while(it->slot < it->map->curSize &&
		        it->map->buf[it->slot].len == 0);
	// advance entry
	} else {
		it->entry++;
	}
	// advance count
	it->count++;
}

/* ----------------------------------------------------------------------------
 * Get the current entry
 * ----------------------------------------------------------------------------
 */
ts_algo_map_slot_t *ts_algo_map_it_get(ts_algo_map_it_t *it) {
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

/* -------------------------------------------------------------------------
 * Convert map to list.
 * -------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_map_toList(ts_algo_map_t *map) {
	ts_algo_rc_t rc = TS_ALGO_OK;
	ts_algo_list_t *l = malloc(sizeof(ts_algo_list_t));
	if (l == NULL) return NULL;
	ts_algo_list_init(l);
	ts_algo_map_it_t *it = ts_algo_map_iterate(map);
	if (it == NULL) {
		free(l); return NULL;
	}
	for(;!ts_algo_map_it_eof(it);ts_algo_map_it_advance(it)) {
		slot_t *s = ts_algo_map_it_get(it);
		if (s == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		rc = ts_algo_list_insert(l, s);
		if (rc != TS_ALGO_OK) break;
	}
	free(it);
	if (rc != TS_ALGO_OK) {
		ts_algo_list_destroy(l); free(l);
		return NULL;
	}
	return l;
}

/* ----------------------------------------------------------------------------
 * Debug: Show size of slots
 * ----------------------------------------------------------------------------
 */
void ts_algo_map_showslots(ts_algo_map_t *map) {
	for (int i=0;i<map->curSize;i++) {
		fprintf(stderr, "%06d: %d\n", i, map->buf[i].len);
	}
}
