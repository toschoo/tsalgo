/* ========================================================================
 * Test linked lists
 * -----------------
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 */
#include <tsalgo/list.h>
#include <tsalgo/random.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ELEMENTS 1024

uint64_t buf[ELEMENTS];

void init_keys() {
	int i;
	for(i=0;i<ELEMENTS;i++) {
		buf[i]=randomUnsigned(1,1000000);
	}
}

ts_algo_bool_t test_simple() {
	ts_algo_list_node_t *tmp;
	ts_algo_list_t list;
	uint64_t key = 1;

	fprintf(stdout, "simple tests\n");

	/* check empty */
	ts_algo_list_init(&list);
	if (list.len != 0) return FALSE;
	if (list.head != list.last ||
	    list.head != NULL) return FALSE;

	/* remove NULL */
	ts_algo_list_remove(&list, list.head);
	if (list.len != 0) return FALSE;
	if (list.head != list.last ||
	    list.last != NULL) return FALSE;

	/* insert 1 */
	ts_algo_list_insert(&list, &key);
	if (list.len != 1) return FALSE;
	if (list.head != list.last ||
	    list.last == NULL) return FALSE;

	/* insert 2 */
	ts_algo_list_insert(&list, &key);
	if (list.len != 2) return FALSE;
	if (list.head == list.last ||
	    list.head == NULL      ||
	    list.last == NULL) return FALSE;

	/* remove to 1 */
	tmp = list.head;
	ts_algo_list_remove(&list, list.head);
	free(tmp);
	if (list.len != 1) return FALSE;
	if (list.head != list.last ||
	    list.last == NULL) return FALSE;

	/* remove to 0 */
	tmp = list.head;
	ts_algo_list_remove(&list, list.head);
	free(tmp);
	if (list.len != 0) return FALSE;
	if (list.head != list.last ||
	    list.last != NULL) return FALSE;

	/* insert 1 */
	ts_algo_list_insert(&list, &key);
	if (list.len != 1) return FALSE;
	if (list.head != list.last ||
	    list.last == NULL) return FALSE;

	/* remove to 0 */
	tmp = list.head;
	ts_algo_list_remove(&list, list.head);
	free(tmp);
	if (list.len != 0) return FALSE;
	if (list.head != list.last ||
	    list.last != NULL) return FALSE;

	/* insert 1 */
	ts_algo_list_insert(&list, &key);
	if (list.len != 1) return FALSE;
	if (list.head != list.last ||
	    list.last == NULL) return FALSE;

	/* insert 2 */
	ts_algo_list_insert(&list, &key);
	if (list.len != 2) return FALSE;
	if (list.head == list.last ||
	    list.head == NULL      ||
	    list.last == NULL) return FALSE;

	/* remove to 1 */
	tmp = list.head;
	ts_algo_list_remove(&list, list.head);
	free(tmp);
	if (list.len != 1) return FALSE;
	if (list.head != list.last ||
	    list.last == NULL) return FALSE;

	/* remove to 0 */
	tmp = list.head;
	ts_algo_list_remove(&list, list.head);
	free(tmp);
	if (list.len != 0) return FALSE;
	if (list.head != list.last ||
	    list.last != NULL) return FALSE;

	ts_algo_list_destroy(&list);
	return TRUE;
}

ts_algo_bool_t test_promote() {
	ts_algo_list_node_t *runner;
	ts_algo_list_t list;
	uint64_t one = 1;
	uint64_t two = 2;
	uint64_t thr = 3;
	uint64_t cnt = 0;
	uint64_t *cur = 0;

	fprintf(stdout, "promotion tests\n");

	ts_algo_list_init(&list);

	ts_algo_list_append(&list, &one);
	ts_algo_list_append(&list, &two);
	ts_algo_list_append(&list, &thr);

	for(runner=list.head;runner!=NULL;runner=runner->nxt) {
		cnt++; cur = runner->cont;
		fprintf(stderr, "%lu ", *cur); 
		if (*cur != cnt) return FALSE;
	}
	fprintf(stderr, "\n");

	/* promote 3: 1 3 2 */
	runner = list.head->nxt->nxt;
	ts_algo_list_promote(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;

	/* promote 3: 3 1 2 */
	runner = list.head->nxt;
	ts_algo_list_promote(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;

	/* promote 2: 3 2 1 */
	runner = list.head->nxt->nxt;
	ts_algo_list_promote(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;

	/* degrade 3: 2 3 1 */
	runner = list.head;
	ts_algo_list_degrade(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;

	/* degrade 3: 2 1 3 */
	runner = list.head->nxt;
	ts_algo_list_degrade(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;

	/* degrade 2: 1 2 3 */
	runner = list.head;
	ts_algo_list_degrade(&list, runner);

	if (list.len != 3) return FALSE;
	
	runner = list.head;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 1) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 2) return FALSE;
	runner = runner->nxt;
	cur = runner->cont;
	fprintf(stderr, "%lu ", *cur);
	if (*cur != 3) return FALSE;

	fprintf(stderr, "\n");

	runner = runner->nxt;
	if (runner != NULL) return FALSE;
	
	ts_algo_list_destroy(&list);
	return TRUE;
}

ts_algo_bool_t test_insert() {
	ts_algo_list_t list;
	ts_algo_list_node_t *runner;
	uint64_t *key;
	int i;

	ts_algo_list_init(&list);
	for(i=ELEMENTS-1;i>=0;i--) {
		if (ts_algo_list_insert(&list,buf+i) != 0)
		{
			ts_algo_list_destroy(&list);
			return FALSE;
		}
	}
	runner=list.head;
	if (runner==NULL) {
		ts_algo_list_destroy(&list);
		return FALSE;
	}
	i=0;
	while(runner!=NULL) {
		key=runner->cont;
		if (*key!=buf[i]) {
			printf("%lu != %lu\n",*key,buf[i]);
			ts_algo_list_destroy(&list);
			return FALSE;
		}
		runner=runner->nxt;i++;
	}
	ts_algo_list_destroy(&list);
	return TRUE;
}

ts_algo_bool_t test_append() {
	ts_algo_list_t list;
	ts_algo_list_node_t *runner;
	uint64_t *key;
	int i;

	ts_algo_list_init(&list);
	for(i=0;i<ELEMENTS;i++) {
		if (ts_algo_list_append(&list,buf+i) != 0)
		{
			ts_algo_list_destroy(&list);
			return FALSE;
		}
	}
	runner=list.head;i=0;
	if(runner==NULL) {
		ts_algo_list_destroy(&list);
		return FALSE;
	}
	while(runner!=NULL) {
		key=runner->cont;
		if (*key!=buf[i]) {
			printf("%lu != %lu\n",*key,buf[i]);
			ts_algo_list_destroy(&list);
			return FALSE;
		}
		runner=runner->nxt;i++;
	}
	ts_algo_list_destroy(&list);
	return TRUE;
}

ts_algo_bool_t test_remove() {
	ts_algo_list_t list;
	ts_algo_list_node_t *runner,*tmp;
	uint64_t *key,k1,k2;
	int i,j,z;

	for(z=0;z<2;z++) {
		ts_algo_list_init(&list);
		if(z==0) {
			for(i=0;i<ELEMENTS;i++) {
				if (ts_algo_list_append(&list,buf+i) != 0)
				{
					ts_algo_list_destroy(&list);
					return FALSE;
				}
			}
		} else {
			init_keys();
			for(i=ELEMENTS-1;i>=0;i--) {
				if (ts_algo_list_insert(&list,buf+i) != 0)
				{
					ts_algo_list_destroy(&list);
					return FALSE;
				}
			}
		}
		for(j=0;j<10;j++) {
			i=randomUnsigned(0,ELEMENTS);
			buf[i]=0;
			if (i==0) printf("head tested!\n");
			if (i==ELEMENTS-1) printf("last tested!\n");
		}
		runner=list.head;
		while(runner!=NULL) {
			key=runner->cont;
			if(*key==0) {
				tmp=runner->nxt;
				ts_algo_list_remove(&list,runner);
				free(runner); runner=tmp;
				continue;
			}
			runner=runner->nxt;
		}
		runner=list.head;i=0;
		if (runner==NULL) {
			ts_algo_list_destroy(&list);
			return FALSE;
		}
		while(runner!=NULL) {
			key=runner->cont;
			if (buf[i] == 0) {
				i++;continue;
			}
			if (*key!=buf[i]) {
				ts_algo_list_destroy(&list);
				return FALSE;
			}
			runner=runner->nxt;i++;
		}
		i=randomUnsigned(0,ELEMENTS-1);
		if (ts_algo_list_insert(&list,buf+i) != 0) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		k1=buf[i];
		i=randomUnsigned(0,ELEMENTS-1);
		if (ts_algo_list_append(&list,buf+i) != 0) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		k2=buf[i];
		runner=list.head;
		if (runner==NULL) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		key=list.head->cont;
		if (*key!=k1) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		runner=runner->nxt;i=0;
		while(runner!=list.last) {
			key=runner->cont;
			if (buf[i]==0) {
				i++;continue;
			}
			if (*key!=buf[i]) {
				ts_algo_list_destroy(&list); return FALSE;
			}
			runner=runner->nxt;i++;
		}
		if (runner==NULL) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		key=runner->cont;
		if (*key!=k2) {
			ts_algo_list_destroy(&list); return FALSE;
		}
		ts_algo_list_destroy(&list);
	}
	return TRUE;
}

ts_algo_bool_t test_reverse() {
	ts_algo_list_t list,*r;
	ts_algo_list_node_t *run1,*run2;
	uint64_t *k1,*k2;
	int i;

	ts_algo_list_init(&list);
	for(i=0;i<ELEMENTS;i++) {
		if (ts_algo_list_append(&list,buf+i) != 0)
		{
			ts_algo_list_destroy(&list);
			return FALSE;
		}
	}
	r = ts_algo_list_reverse(&list);
	if (r == NULL) {
		printf("cannot reverse\n");
		ts_algo_list_destroy(&list);
		return FALSE;
	}
	run1=list.head;run2=r->last;
	while(run1!=NULL && run2!=NULL) {
		k1=run1->cont;
		k2=run2->cont;
		if (*k1 != *k2) {
			printf("reverse: %llu != %llu\n",
			       (unsigned long long)(*k1),
			       (unsigned long long)(*k2));
			ts_algo_list_destroy(&list);
			ts_algo_list_destroy(r); free(r);
			return FALSE;
		}
		run1=run1->nxt;run2=run2->prv;
	}
	if (run1!=run2) {
		printf("reverse: lists are not of equal length\n");
		ts_algo_list_destroy(&list);
		ts_algo_list_destroy(r); free(r);
		return FALSE;
	}
	ts_algo_list_destroy(&list);
	ts_algo_list_destroy(r); free(r);
	return TRUE;
}

ts_algo_cmp_t mycompare(uint64_t *k1, uint64_t *k2) {
	if (*k1 < *k2) return ts_algo_cmp_less;
	if (*k1 > *k2) return ts_algo_cmp_greater;
	return ts_algo_cmp_equal;
}

ts_algo_bool_t validate(ts_algo_list_t *list) {
	uint64_t *k1,*k2;
	ts_algo_list_node_t *runner;

	runner = list->head;
	if (runner != NULL) {
		k1 = list->head->cont;
		runner=runner->nxt;
	}
	while(runner != NULL) {
		k2 = runner->cont;
		if (*k1 > *k2) {
			printf("%llu > %llu\n", 
			      (unsigned long long)(*k1),
			      (unsigned long long)(*k2));
			return FALSE;
		}
		k1 = k2;
		runner = runner->nxt;
	}
	return TRUE;
}

ts_algo_bool_t test_sort() {
	ts_algo_list_t list,*sorted;
	int i;
	ts_algo_bool_t rc;

	ts_algo_list_init(&list);
	for(i=0;i<ELEMENTS;i++) {
		if (ts_algo_list_append(&list,buf+i) != 0)
		{
			ts_algo_list_destroy(&list);
			return FALSE;
		}
	}
	sorted = ts_algo_list_sort(&list,(ts_algo_compare_t)&mycompare);
	if (sorted == NULL) {
		printf("cannot sort!\n");
		ts_algo_list_destroy(&list);
		return FALSE;
	}
	if (sorted->len != list.len) {
		printf("lists differ: %lu - %lu\n",
		        (unsigned long)list.len, 
			(unsigned long)sorted->len);
		ts_algo_list_destroy(&list);
		ts_algo_list_destroy(sorted);
		return FALSE;
	}
	rc = validate(sorted);
	ts_algo_list_destroy(&list);
	ts_algo_list_destroy(sorted);free(sorted);
	return rc;
}

ts_algo_bool_t test_merge() {
	ts_algo_list_t l1,l2,*s1,*s2,*m;
	int i;
	ts_algo_bool_t rc;

	ts_algo_list_init(&l1);
	ts_algo_list_init(&l2);
	for(i=0;i<ELEMENTS;i++) {
		if (ts_algo_list_append(&l1,buf+i) != 0)
		{
			ts_algo_list_destroy(&l1);
			ts_algo_list_destroy(&l2);
			return FALSE;
		}
		if (ts_algo_list_insert(&l2,buf+i) != 0)
		{
			ts_algo_list_destroy(&l1);
			ts_algo_list_destroy(&l2);
			return FALSE;
		}
	}
	s1 = ts_algo_list_sort(&l1,(ts_algo_compare_t)&mycompare);
	if (s1 == NULL) {
		printf("cannot sort!\n");
		ts_algo_list_destroy(&l1);
		ts_algo_list_destroy(&l2);
		return FALSE;
	}
	s2 = ts_algo_list_sort(&l2,(ts_algo_compare_t)&mycompare);
	if (s2 == NULL) {
		printf("cannot sort!\n");
		ts_algo_list_destroy(&l1);
		ts_algo_list_destroy(&l2);
		ts_algo_list_destroy(s1);free(s1);
		return FALSE;
	}
	m = ts_algo_list_merge(s1,s2,(ts_algo_compare_t)&mycompare);
	if (m == NULL) {
		printf("cannot merge!\n");
		ts_algo_list_destroy(&l1);
		ts_algo_list_destroy(&l2);
		ts_algo_list_destroy(s1);free(s1);
		ts_algo_list_destroy(s2);free(s2);
		return FALSE;
	}
	if (m->len != l1.len + l2.len) {
		printf("lists differ: %lu - %lu\n",
		        (unsigned long)l1.len + l2.len, 
			(unsigned long)m->len);
		ts_algo_list_destroy(&l1);
		ts_algo_list_destroy(&l2);
		ts_algo_list_destroy(s1);free(s1);
		ts_algo_list_destroy(s2);free(s2);
		ts_algo_list_destroy(m);free(m);
		return FALSE;
	}
	rc = validate(m);
	ts_algo_list_destroy(&l1);
	ts_algo_list_destroy(&l2);
	ts_algo_list_destroy(s1);free(s1);
	ts_algo_list_destroy(s2);free(s2);
	ts_algo_list_destroy(m);free(m);
	return rc;
}

int main () {
	int i;
	init_rand();

	if (!test_simple()) {
		printf("test simple failed\n");
		return EXIT_FAILURE;
	}

	if (!test_promote()) {
		printf("test promote failed\n");
		return EXIT_FAILURE;
	}

	for (i=0;i<100;i++) {
		init_keys();
		if (!test_insert()) {
			printf("test insert failed\n");
			return EXIT_FAILURE;
		}
	}
	for (i=0;i<100;i++) {
		init_keys();
		if (!test_append()) {
			printf("test append failed\n");
			return EXIT_FAILURE;
		}
	}
	for (i=0;i<100;i++) {
		init_keys();
		if (!test_remove()) {
			printf("test remove failed\n");
			return EXIT_FAILURE;
		}
	}
	for (i=0;i<100;i++) {
		init_keys();
		if (!test_reverse()) {
			printf("test reverse failed\n");
			return EXIT_FAILURE;
		}
	}
	for (i=0;i<100;i++) {
		init_keys();
		if (!test_sort()) {
			printf("test sort failed\n");
			return EXIT_FAILURE;
		}
	}
	for (i=0;i<100;i++) {
		init_keys();
		if (!test_merge()) {
			printf("test sort failed\n");
			return EXIT_FAILURE;
		}
	}
	printf("all test passed!\n");
	return EXIT_SUCCESS;
}
