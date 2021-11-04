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
		int k = 1000+i;
		buf[i].key = k;
		sprintf(buf[i].name, "%d", k);
	}
}

char keyseen(uint64_t *mem, int sz, uint64_t k) {
	for(int i=0; i<sz; i++) {
		if (k == mem[i]) return 1;
	}
	return 0;
}

int testAddAndIter(uint32_t mapsz) {
	uint64_t *mem = NULL;
	char err = 0;
	srand(time(NULL));
	bufinit();
	ts_algo_map_t *map = ts_algo_map_new(mapsz, ts_algo_hash_id, NULL);
	if (map == NULL) {
		fprintf(stderr, "Can't creat map\n");
		return -1;
	}
	for (int i=0;i<BUFSZ;i++) {
		ts_algo_rc_t rc = ts_algo_map_addId(map, buf[i].key, buf+i);
		if (rc != TS_ALGO_OK) {
			fprintf(stderr, "Can't add: %d\n", rc);
			err = 1;
			goto cleanup;
		}
	}
	// ts_algo_map_showslots(map); fprintf(stderr, "\n");

	ts_algo_map_it_t *it=NULL;
	for(int i=0;i<2;i++) {
		if (i==0) {
			fprintf(stderr, "Iterator...\n");
			it=ts_algo_map_iterate(map);
		} else {
			fprintf(stderr, "Once more...\n");
			ts_algo_map_it_reset(it);
		}
		int x=0;
		for(; // call ts_algo_map_iterate here
		    it != NULL && !ts_algo_map_it_eof(it);
		    ts_algo_map_it_advance(it)
		) {
			ts_algo_map_slot_t *s = ts_algo_map_it_get(it);
			if (s == NULL) {
				fprintf(stderr, "NO SLOT\n");
				free(it); it=NULL; break;
			}
			// fprintf(stdout, "%d: %lu -> '%s'\n", it->count-1, (uint64_t)(*s->key), (char*)((mydata_t*)s->data)->name);
			x++;
			if (x != it->count) {
				free(it); it=NULL; break;
			}
		}
		if (it == NULL) {
			fprintf(stderr, "Can't iterate\n");
			err = 1;
			goto cleanup;
		} else {
			if (it->count != map->count+1) {
				fprintf(stderr, "wrong count: %d | %d\n", it->count, map->count);
				err = 1;
				free(it);
				goto cleanup;
			}
			if (i!=0) free(it);
		}
	    
		fprintf(stderr, "...OK\n");
	}
	mem = calloc(10, sizeof(uint64_t));
	if (mem == NULL) {
		fprintf(stderr, "No more memory\n");
		err = 1;
		goto cleanup;
	}
	fprintf(stderr, "Random search...\n");
	for(int i=0;i<10;i++) {
		int k;
		do {
			k=rand()%BUFSZ;
		} while (keyseen(mem, i, k));
		mem[i] = k;
		mydata_t *data = ts_algo_map_getId(map, buf[k].key);
		// fprintf(stderr, "looking at '%s'\n", buf[k].name);
		if (strcmp(data->name, buf[k].name) != 0) {
			fprintf(stderr, "FAIL: wrong name for %d: %s\n", k, data->name);
			err = 1;
			goto cleanup;
		}
		mydata_t *datb = ts_algo_map_removeId(map, buf[k].key);
		if (datb != data) {
			fprintf(stderr, "FAIL: something awful happened: %p != %p\n", data, datb);
			err = 1;
			goto cleanup;
		}
		data = ts_algo_map_getId(map, buf[k].key);
		if (data != NULL) {
			fprintf(stderr, "FAIL: found removed element: %p (%p)\n", data, datb);
			err = 1;
			goto cleanup;
		}
	}
	fprintf(stderr, "...OK\n");
cleanup:
	if (mem != NULL) free(mem);
	ts_algo_map_destroy(map); free(map);
	if (err == 0) {
		fprintf(stderr, "SUCCESS!\n");
		return 0;
	}
	return -1;
}

int main() {
	for(int i=0;i<100;i++) {
		if (testAddAndIter(0) != 0) exit(1);
		if (testAddAndIter(256) != 0) exit(1);
		if (testAddAndIter(32) != 0) exit(1);
	}
	exit(0);
}
