#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <tsalgo/map.h>
#include <tsalgo/random.h>
#include <progress.h>

typedef struct timespec timestamp_t;
int timestamp(timestamp_t *tmstp) {
	return clock_gettime(CLOCK_MONOTONIC, tmstp);
}

#define NPERSEC 1000000000
#define ELEMENTS 100000

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
	uint64_t *buf = malloc(sizeof(uint64_t)*ELEMENTS);
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

char testinsmap(int it) {
	progress_t p;
	timestamp_t t1,t2;
	uint64_t d = 0;
	char err = 0;
	ts_algo_map_t *map = malloc(sizeof(ts_algo_map_t));
	if (map == NULL) {
		fprintf(stderr, "Can't creat map\n");
		return -1;
	}
	fprintf(stderr, "Map\n");
	init_progress(&p,stdout,it);
	for (int j=0;j<it;j++) {
		ts_algo_map_init(map, 32168, NULL);
		if (timestamp(&t1)) {
			printf("cannot timestamp\n");
			err = 1;
			break;
		}
		for (int i=0;i<ELEMENTS;i++) {
			ts_algo_rc_t rc = ts_algo_map_add(map, i, (void*)(uint64_t)i);
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
	if (testinsbuf(100) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	} 
	if (testinsmap(100) != 0) {
		fprintf(stderr, "FAILED!\n");
		exit(1);
	} 
	fprintf(stderr, "PASSED!\n");
	return 0;
}
