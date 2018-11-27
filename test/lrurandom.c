/* ========================================================================
 * Test LRU
 * --------
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <tsalgo/random.h>
#include <tsalgo/lru.h>
#include <progress.h>

#define ELEMENTS 4096
#define STRLEN   8

typedef struct {
	int     idx;
	char str[9];
        char rsdnt;
} map_t;

ts_algo_cmp_t byname(void *ignore, map_t *left, map_t *right) {
	int x = strcmp(left->str, right->str);
	if (x < 0) return ts_algo_cmp_less;
	if (x > 0) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

ts_algo_cmp_t byidx(void *ignore, map_t *left, map_t *right) {
	if (left->idx < right->idx) return ts_algo_cmp_less;
	if (left->idx > right->idx) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

ts_algo_rc_t update(void *ignore, void *oldN, void *newN) {
	return TS_ALGO_OK;
}

void nodestroy(void *ignore, void **x) {}

void destroy(void *ignore, map_t **cont) {
	/*
	fprintf(stderr, "destroying %d->%s\n", (*cont)->idx, (*cont)->str);
	*/
	free(*cont); *cont=NULL;
}

void randomstring(char *str) {
	for(int i=0; i<STRLEN; i++) {
		str[i] = rand()%26;
		str[i]+=64;
	}
	str[STRLEN]=0;
}

map_t *makeElements() {
	map_t *buf = malloc(sizeof(map_t)*ELEMENTS);
	if (buf == NULL) return NULL;

	for(int i=0;i<ELEMENTS;i++) {
		buf[i].idx = i;
		randomstring(buf[i].str);
		buf[i].rsdnt = 0;
	}
	return buf;
}

ts_algo_rc_t addandget() {
	ts_algo_lru_t  lru;
	ts_algo_rc_t rc=TS_ALGO_OK;
	map_t  *mymap=NULL;
	map_t  *found=NULL;

	rc = ts_algo_lru_init(&lru,0,
	     (ts_algo_comprsc_t)&byname,
	     (ts_algo_update_t)&update,
	     (ts_algo_delete_t)&destroy,
	     (ts_algo_delete_t)&destroy);
	if (rc != TS_ALGO_OK) {
		return rc;
	}

	mymap = malloc(sizeof(map_t));
	if (mymap == NULL) {
		rc = TS_ALGO_NO_MEM;
		goto cleanup;
	}
	mymap->idx = 42;
	strcpy(mymap->str, "random");

	rc = ts_algo_lru_add(&lru, mymap);
	if (rc != TS_ALGO_OK) {
		free(mymap); goto cleanup;
	}
	found = ts_algo_lru_get(&lru, mymap);
	if (found == NULL) {
		rc = TS_ALGO_ERR; goto cleanup;
	}
	if (found->idx != 42) {
		rc = TS_ALGO_ERR; goto cleanup;
	}
	if (strcmp(found->str, "random") != 0) {
		rc = TS_ALGO_ERR; goto cleanup;
	}
	
cleanup:
	ts_algo_lru_destroy(&lru);
	return rc;
}

ts_algo_rc_t byidxKilo() {
	map_t  *mymap=NULL;
	map_t  *found=NULL;
	ts_algo_rc_t rc = TS_ALGO_OK;
	ts_algo_lru_t lru;
	map_t *buf = makeElements();
	if (buf == NULL) return TS_ALGO_NO_MEM;

	rc = ts_algo_lru_init(&lru,100,
	     (ts_algo_comprsc_t)&byidx,
	     (ts_algo_update_t)&update,
	     (ts_algo_delete_t)&nodestroy,
	     (ts_algo_delete_t)&nodestroy);
	if (rc != TS_ALGO_OK) {
		free(buf); return rc;
	}

	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}
	if (rc != TS_ALGO_OK) goto cleanup;
	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}

cleanup:
	free(buf);
	ts_algo_lru_destroy(&lru);
	return rc;
}

ts_algo_rc_t bynameKilo() {
	map_t  *mymap=NULL;
	map_t  *found=NULL;
	ts_algo_rc_t rc = TS_ALGO_OK;
	ts_algo_lru_t lru;
	map_t *buf = makeElements();
	if (buf == NULL) return TS_ALGO_NO_MEM;

	rc = ts_algo_lru_init(&lru,100,
	     (ts_algo_comprsc_t)&byname,
	     (ts_algo_update_t)&update,
	     (ts_algo_delete_t)&nodestroy,
	     (ts_algo_delete_t)&nodestroy);
	if (rc != TS_ALGO_OK) {
		free(buf); return rc;
	}

	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}
	if (rc != TS_ALGO_OK) goto cleanup;
	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}

cleanup:
	free(buf);
	ts_algo_lru_destroy(&lru);
	return rc;
}

ts_algo_rc_t byidxResidents() {
	map_t  *mymap=NULL;
	map_t  *found=NULL;
	ts_algo_rc_t rc = TS_ALGO_OK;
	ts_algo_lru_t lru;
	int rvk=0, fnd=0;

	map_t *buf = makeElements();
	if (buf == NULL) return TS_ALGO_NO_MEM;

	for(int i=0; i<100; i++) {
		buf[i].rsdnt = 1;
	}

	rc = ts_algo_lru_init(&lru,50,
	     (ts_algo_comprsc_t)&byidx,
	     (ts_algo_update_t)&update,
	     (ts_algo_delete_t)&nodestroy,
	     (ts_algo_delete_t)&nodestroy);
	if (rc != TS_ALGO_OK) {
		free(buf); return rc;
	}
	for(int i=0; i<100; i++) {
		mymap = buf+i;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_addResident(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			rc = TS_ALGO_ERR; break;
		}
		if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}
	fprintf(stderr, "lru size 1: %d\n", lru.list.len);
	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) {
				fprintf(stderr, "cannot add\n");
			}
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			if (lru.list.len < lru.max) {
				rc = TS_ALGO_ERR; break;
			}

		} else if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}
	for(int i=0; i<100; i++) {
		mymap = buf+i;
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			fprintf(stderr, "resident %d not found\n", i);
			rc = TS_ALGO_ERR; break;

		} else if (found->idx != mymap->idx ||
		    strcmp(found->str, mymap->str) != 0) {
			rc = TS_ALGO_ERR; break;
		}
	}
	rvk=0;
	for(int i=0; i<100; i++) {
		mymap = buf+i;
		int x = rand()%10;
		if (x != 0) {
			rvk++;
			mymap->rsdnt = 0;
			ts_algo_lru_revokeResidence(&lru, mymap);
		}
	}
	fprintf(stderr, "lru size 2: %d\n", lru.list.len);
	if (rc != TS_ALGO_OK) goto cleanup;
	for(int i=0;i<1000;i++) {
		int x = rand()%4096;
		mymap = buf+x;
		found = ts_algo_lru_get(&lru, mymap);
		if (found != NULL) {
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			rc = ts_algo_lru_add(&lru, mymap);
			if (rc != TS_ALGO_OK) break;
		}
		found = ts_algo_lru_get(&lru, mymap);
		if (found == NULL) {
			if (lru.list.len < lru.max) {
				rc = TS_ALGO_ERR; break;
			}
		} else {
			fnd++;
			if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		}
	}
	if (rc != TS_ALGO_OK) goto cleanup;
	if (fnd < 1000) {
		fprintf(stderr, "no room despite revocation: %d / %d\n",
		                                              rvk, fnd);
		rc = TS_ALGO_ERR; goto cleanup;
	}
	fprintf(stderr, "lru size 3: %d\n", lru.list.len);
	fprintf(stderr, "revoked / found: %d / %d\n", rvk, fnd);
	fnd=0;
	for(int i=0; i<100; i++) {
		mymap = buf+i;
		if (mymap->rsdnt) {
			fnd++;
			found = ts_algo_lru_get(&lru, mymap);
			if (found == NULL) {
				fprintf(stderr, "resident %d not found\n", i);
				rc = TS_ALGO_ERR; break;

			} else if (found->idx != mymap->idx ||
			    strcmp(found->str, mymap->str) != 0) {
				rc = TS_ALGO_ERR; break;
			}
		}
	}
	if (fnd != 100-rvk) {
		fprintf(stderr, "Some residents lost: %d / %d\n",
		                                    fnd, 100-rvk);
		rc = TS_ALGO_ERR; goto cleanup;
	}

cleanup:
	free(buf);
	ts_algo_lru_destroy(&lru);
	return rc;
}

int main() {
	ts_algo_rc_t rc;
	srand((uint64_t)time(NULL) & (uint64_t)&printf);

	fprintf(stderr, "Testing LRU Cache\n");

	/* simple... */
	rc = addandget();
	if (rc != TS_ALGO_OK) {
		fprintf(stderr, "simple failed: %d\n", rc);
		goto failure;
	}

	/* 1000/4096 */
	rc = byidxKilo();
	if (rc != TS_ALGO_OK) {
		fprintf(stderr, "byidx failed: %d\n", rc);
		goto failure;
	}

	/* 1000/4096 */
	rc = bynameKilo();
	if (rc != TS_ALGO_OK) {
		fprintf(stderr, "bystr failed: %d\n", rc);
		goto failure;
	}

	/* 1000/4096/residents */
	rc = byidxResidents();
	if (rc != TS_ALGO_OK) {
		fprintf(stderr, "residents failed: %d\n", rc);
		goto failure;
	}
	fprintf(stdout, "PASSED\n");
	return EXIT_SUCCESS;

failure:
	fprintf(stdout, "FAILED\n");
	return EXIT_FAILURE;
}

