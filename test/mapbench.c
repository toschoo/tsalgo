#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <tsalgo/map.h>
#include <tsalgo/random.h>
#include <progress.h>
#ifdef __cplusplus
}
#endif

#ifdef _CITY_
#include <city.h>
uint64_t Hash64(const char *key, size_t ksz, void *ignore) {
	return CityHash64(key, ksz);
}
#endif

typedef struct timespec timestamp_t;
int timestamp(timestamp_t *tmstp) {
	return clock_gettime(CLOCK_MONOTONIC, tmstp);
}

#define NPERSEC 1000000000
#define ELEMENTS 100000

char *mystrings[ELEMENTS];

char *randomstring() {
	char *s = (char*)malloc(9);
	if (s == NULL) return NULL;
	s[8] = 0;
	for (int i=0;i<8;i++) {
		s[i] = rand()%26 + 65;
	}
	// fprintf(stderr, "%s\n", s);
	return s;
}

int initstringbuf() {
	for(int i=0;i<ELEMENTS;i++) {
		mystrings[i] = randomstring();
		if (mystrings[i] == NULL) return -1;
	}
	return 0;
}

void destroystringbuf() {
	for(int i=0;i<ELEMENTS;i++) {
		if (mystrings[i] != NULL) free(mystrings[i]);
	}
}

uint64_t timediff(timestamp_t *t1, timestamp_t *t2) {
	timestamp_t t3; /* t3 = t1 - t2 */

	if (t2->tv_nsec > t1->tv_nsec) {
		t3.tv_nsec = 
			(NPERSEC + t1->tv_nsec) - t2->tv_nsec;
		t3.tv_sec = (t1->tv_sec - 1) - t2->tv_sec;
	} else {
		t3.tv_nsec = t1->tv_nsec - t2->tv_nsec;
		t3.tv_sec  = t1->tv_sec  - t2->tv_sec;
	}
	return (t3.tv_sec * NPERSEC + t3.tv_nsec);
}

char testinsbuf(int it) {
	progress_t p;
	timestamp_t t1,t2;
	uint64_t d = 0;
	char err = 0;
	uint64_t *buf = (uint64_t*)malloc(sizeof(uint64_t)*ELEMENTS);
	if (buf == NULL) {
		fprintf(stderr, "Can't create buf\n");
		return -1;
	}
	fprintf(stderr, "Ordinary Buf\n");
	init_progress(&p,stdout,it);
	for (int j=0;j<it;j++) {
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		for (int i=0;i<ELEMENTS;i++) {
			buf[i] = (uint64_t)i;
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			return FALSE;
		}
		d += timediff(&t2,&t1);
		update_progress(&p,j);
	}
	close_progress(&p);printf("\n");
	if (err != 0) goto cleanup;
	fprintf(stderr, "Average: %ldus for %d elements\n", (d/it)/1000, ELEMENTS);
cleanup:
	free(buf);
	return err;
}

char testinsmap(int it, uint32_t sz, ts_algo_hash_t hsh) {
	char city=0;
	progress_t p;
	timestamp_t t1,t2;
	uint64_t d = 0;
	char err = 0;
	ts_algo_map_t *map = (ts_algo_map_t*)malloc(sizeof(ts_algo_map_t));
	if (map == NULL) {
		fprintf(stderr, "Can't creat map\n");
		return -1;
	}
	if (hsh == (ts_algo_hash_t)ts_algo_hash_id) {
		fprintf(stderr, "Map with Id and base size %u\n", sz);
	} else {
		city=1;
		fprintf(stderr, "Map with Hash64 and base size %u\n", sz);
	}
	init_progress(&p,stdout,it);
	for (int j=0;j<it;j++) {
		if (ts_algo_map_init(map, sz, hsh, NULL) != TS_ALGO_OK) {
			printf("cannot init map\n");
			err = 1;
			break;
		}
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			err = 1;
			break;
		}
		for (uint64_t i=0;i<ELEMENTS;i++) {
			ts_algo_rc_t rc;
			if (city)
				rc = ts_algo_map_add(map, mystrings[i], 8, (void*)(uint64_t)i);
			else 
				rc = ts_algo_map_add(map, (char*)&i, sizeof(uint64_t), (void*)(uint64_t)i);

			if (rc != TS_ALGO_OK) {
				fprintf(stderr, "Can't add: %d\n", rc);
				err = 1; break;
			}
		}
		if (timestamp(&t2)) {
			printf("cannot timestamp\n");
			err = 1;
			break;
		}
		d += timediff(&t2,&t1);
		update_progress(&p,j);
		ts_algo_map_destroy(map);
	}
	close_progress(&p);printf("\n");
	if (err != 0) goto cleanup;
	fprintf(stderr, "Average: %ldus for %d elements\n", (d/it)/1000, ELEMENTS);
	fprintf(stderr, "Size: %d\n", map->count);
	// ts_algo_map_showslots(map);
cleanup:
	if (err != 0) ts_algo_map_destroy(map);
	free(map);
	return err;
}

int main() {
	srand(time(NULL));
	if (initstringbuf() != 0) {
		fprintf(stderr, "out of mem\nFAILED!\n");
		exit(1);
	}
	if (testinsbuf(100) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	} 
	if (testinsmap(100, 32768, ts_algo_hash_id) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	}
	if (testinsmap(100, 65536, ts_algo_hash_id) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	}
#ifdef _CITY_ 
	if (testinsmap(100, 32768, Hash64) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	}
	if (testinsmap(100, 65536, Hash64) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	} 
#endif
	destroystringbuf();
	fprintf(stderr, "PASSED!\n");
	return 0;
}
