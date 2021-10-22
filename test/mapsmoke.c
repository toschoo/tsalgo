#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tsalgo/map.h>

#define BUFSZ 256

typedef struct {
	uint64_t  key;
	char name[32];
} mydata_t;

mydata_t buf[BUFSZ];

void bufinit() {
	for(int i=0;i<BUFSZ;i++) {
		int k = 1000*i;
		buf[i].key = k;
		sprintf(buf[i].name, "%d", k);
	}
}

int main() {
	char err = 0;
	srand(time(NULL));
	bufinit();
	ts_algo_map_t *map = ts_algo_map_new(0, NULL);
	if (map == NULL) {
		fprintf(stderr, "Can't creat map\n");
		exit(1);
	}
	for (int i=0;i<BUFSZ;i++) {
		ts_algo_rc_t rc = ts_algo_map_add(map, buf[i].key, buf+i);
		if (rc != TS_ALGO_OK) {
			fprintf(stderr, "Can't add: %d\n", rc);
			err = 1;
			goto cleanup;
		}
	}
	for(int i=0;i<10;i++) {
		int k=rand()%BUFSZ;
		mydata_t *data = ts_algo_map_get(map, buf[k].key);
		fprintf(stderr, "looking at '%s'\n", buf[k].name);
		if (strcmp(data->name, buf[k].name) != 0) {
			fprintf(stderr, "FAIL: wrong name for %d: %s\n", k, data->name);
			err = 1;
			goto cleanup;
		}
	}
cleanup:
	ts_algo_map_destroy(map); free(map);
	if (err == 0) {
		fprintf(stderr, "SUCCESS!\n");
		exit(0);
	}
	exit(1);
}
