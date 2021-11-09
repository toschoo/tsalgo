/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2021
 * ========================================================================
 * The Map Datatype
 * A hashmap providing callbacks to handle generic data.
 * The map hashes the key data passed in and takes the resulting number
 * modulo the current size of the internal buffer.
 * The hash function is provided by the user application. It should be chosen
 * according to the requirements of the application, such as performance
 * or persistency. Note that some hash functions do not guarantee
 * backward compatibility between releases.
 * Such a hash would be a poor choice where persistency is needed.
 * The library provides an "identity" hash intended for applications
 * where the key can be represented as an unsigend 64bit integer.
 * ========================================================================
 */
#ifndef ts_algo_map_decl
#define ts_algo_map_decl

#include <tsalgo/types.h>
#include <tsalgo/list.h>

/* -------------------------------------------------------------------------
 * Type of a hash function that, provided a byte buffer, returns a 64bit key.
 * The third parameter is intended for additional resources needed by the
 * concrete hash function.
 * -------------------------------------------------------------------------
 */
typedef uint64_t (*ts_algo_hash_t)(const char*,size_t,void*);

/* -------------------------------------------------------------------------
 * The id "hash" that, provided a byte buffer representing an integer,
 * returns a 64bit key that corresponds to the bytes in the buffer
 * interpreted as a 64bit unsigned integer. If 'key' is NULL the result is 0.
 * This function is intended for use with applications that actually
 * use uint64_t keys that do not need further hashing.
 * Note that "not needing further hashing" means more
 * than just being numerical. The keys used by the application should
 * also not all share divisers or be all multiples of each other.
 * Otherwise, only a subset of slots of the hashmap will be used.
 * The third parameter is ignored.
 * -------------------------------------------------------------------------
 */
uint64_t ts_algo_hash_id(const char *key, size_t ksz, void *ignore);

/* -------------------------------------------------------------------------
 * The map structure
 * -------------------------------------------------------------------------
 */
typedef struct {
  uint32_t    baseSize; // Base size defined by the application
  uint32_t     curSize; // Current size, which, after resizing, may differ from the base size
  uint32_t       count; // Number of elements in the map
  ts_algo_hash_t   hsh; // Hash function defined by the application
  ts_algo_delete_t del; // Function to delete data defined by the application
  ts_algo_list_t  *buf; // Internal buffer to store the data
} ts_algo_map_t;

/* -------------------------------------------------------------------------
 * Map Iterator
 * -------------------------------------------------------------------------
 */
typedef struct {
  ts_algo_map_t *map; // The map over which to iterate
  int           slot; // Current slot in the buffer
  int          entry; // Current entry in that slot
  uint32_t     count; // Number of the element we will see next (starting from 0)
} ts_algo_map_it_t;

/* -------------------------------------------------------------------------
 * Slot Structure
 * -------------------------------------------------------------------------
 */
typedef struct {
	char     *key; // The key
	size_t    ksz; // The size of the key data
	uint64_t hash; // The hash of the key
	void    *data; // The data stored under the key
} ts_algo_map_slot_t;

/* -------------------------------------------------------------------------
 * Allocate a new map; for parameters, please refer to ts_algo_map_init.
 * -------------------------------------------------------------------------
 */
ts_algo_map_t *ts_algo_map_new(uint32_t sz,
                      ts_algo_hash_t   hsh,
                      ts_algo_delete_t del);

/* -------------------------------------------------------------------------
 * Initialise an already allocated map; the parameters are
 * - map: The map to initialise
 * - sz : The base size of the map buffer, if sz is 0, the buffer will be
 *        initialised to a default. If the approximate number of elements
 *        is known in advance, setting the size to at least half of that
 *        number can improve performance up to factor 2.
 * - hsh: The hash function; if no hashing is necessary, ts_algo_hash_id
 *        shall be passed.
 * - del: The function used to free the memory of the application data
 *        when deleted or when the map is destroyed.
 *        If the map shall not free that memory, the application shall
 *        pass NULL.
 * -------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_map_init(ts_algo_map_t   *map,
                              uint32_t          sz,
                              ts_algo_hash_t   hsh,
                              ts_algo_delete_t del);

/* -------------------------------------------------------------------------
 * Destroy the map when not needed anymore. 
 * If a 'del' function was supplied it will applied to all data
 * in the map. Otherwise, if 'del' is NULL, life cycle management
 * of the data is left to the application.
 * -------------------------------------------------------------------------
 */
void ts_algo_map_destroy(ts_algo_map_t *map);

/* -------------------------------------------------------------------------
 * Add a key value pair to the map; parameters:
 * - map : The map on which we are operating;
 * - key : The byte buffer holding the key data. The bytes are copyied
 *         into the map structure, so the user does not worry about the
 *         life cycle after adding the pair to the map.
 * - ksz : Size of the key data, e.g.
 *         - in case of a string: the string length, either with or without
 *           the terminating 0 (but consistent for one application)
 *         - in case of a key used with ts_algo_hash_id: sizeof(uint64_t).
 *           Note that the buffer must have 8 bytes in this case!
 * - data: The data to be stored with the key. The life cycle may be
 *         managed by the map (then a del function should be passed)
 *         or by the application (then the del function should be NULL).
 * -------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_map_add(ts_algo_map_t *map, char *key, size_t ksz, void *data);

/* -------------------------------------------------------------------------
 * Obtain the data stored with the key.
 * For parameters please refer to ts_algo_map_add.
 * -------------------------------------------------------------------------
 */
void *ts_algo_map_get(ts_algo_map_t *map, char *key, size_t ksz);

/* -------------------------------------------------------------------------
 * Removes the data stored with the key and returns them to the application.
 * The data are not destroyed!
 * -------------------------------------------------------------------------
 */
void *ts_algo_map_remove(ts_algo_map_t *map, char *key, size_t ksz);

/* -------------------------------------------------------------------------
 * Removes the data stored with the key and, if 'del' is not NULL,
 * calls this function on the data.
 * -------------------------------------------------------------------------
 */
void ts_algo_map_delete(ts_algo_map_t *map, char *key, size_t ksz);

/* -------------------------------------------------------------------------
 * Replaces the data stored with the key by the data passed in.
 * If the key was found, the old data are returned, otherwise NULL is returned
 * and no changes are perfomed. The old data are not destroyed!
 * -------------------------------------------------------------------------
 */
void *ts_algo_map_update(ts_algo_map_t *map, char *key, size_t ksz, void *data);

/* -------------------------------------------------------------------------
 * Convenience interfaces for a map using ts_algo_hash_id.
 * The parameters in all interfaces are:
 * - m: The map on which we are operating;
 * - k: The uint64_t  key;
 * - d: The data.
 * -------------------------------------------------------------------------
 */
#define ts_algo_map_addId(m,k,d) \
	ts_algo_map_add(m, (char*)&k, sizeof(uint64_t), d);

#define ts_algo_map_getId(m,k) \
	ts_algo_map_get(m, (char*)&k, sizeof(uint64_t));

#define ts_algo_map_removeId(m,k) \
	ts_algo_map_remove(m, (char*)&k, sizeof(uint64_t));

#define ts_algo_map_deleteId(m,k) \
	ts_algo_map_delete(m, (char*)&k, sizeof(uint64_t));

#define ts_algo_map_updateId(m,k,d) \
	ts_algo_map_update(m, (char*)&k, sizeof(uint64_t), d);

/* -------------------------------------------------------------------------
 * Create an iterator for the map.
 * Iterators are used according to the following recipe:
 * 
 * ts_algo_it_t *it = ts_algo_map_iterate(map);
 * if (it == NULL) error handling ...
 * for(;!ts_algo_it_eof(it);ts_algo_it_advance(it)) {
 *   ts_algo_slot_t *s = ts_algo_it_get(it);
 *   if (s == NULL) break; // something is wrong!
 *   // do something with the data
 * }
 * free(it);
 * 
 * Note that the order of elements as presented by the iterator is
 *      arbitrary. Application code should rely on any specific ordering.
 * -------------------------------------------------------------------------
 */
ts_algo_map_it_t *ts_algo_map_iterate(ts_algo_map_t *map);

/* -------------------------------------------------------------------------
 * Advance the iterator.
 * -------------------------------------------------------------------------
 */
void ts_algo_map_it_advance(ts_algo_map_it_t *it);

/* -------------------------------------------------------------------------
 * Check if the iterator reached the end of the map.
 * -------------------------------------------------------------------------
 */
char ts_algo_map_it_eof(ts_algo_map_it_t *it);

/* -------------------------------------------------------------------------
 * Rewind the iterator.
 * -------------------------------------------------------------------------
 */
void ts_algo_map_it_rewind(ts_algo_map_it_t *it);

/* -------------------------------------------------------------------------
 * Get the current slot.
 * -------------------------------------------------------------------------
 */
ts_algo_map_slot_t *ts_algo_map_it_get(ts_algo_map_it_t *it);

/* -------------------------------------------------------------------------
 * Convert map to list. The list contains the original slots from the map.
 * The memory should not be freed, the slots are still pointed to
 * by the original map. Note further that the list is not sorted.
 * -------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_map_toList(ts_algo_map_t *map);

/* -------------------------------------------------------------------------
 * Debug function that shows the number of entries per slot.
 * -------------------------------------------------------------------------
 */
void ts_algo_map_showslots(ts_algo_map_t *map);

#endif
