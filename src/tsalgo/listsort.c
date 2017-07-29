/* ========================================================================
 * (c) Tobias Schoofs, 2016
 * ========================================================================
 * ListSort
 * ========================================================================
 * Mergesort on lists
 * ========================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <tsalgo/types.h>
#include <tsalgo/list.h>

/* ------------------------------------------------------------------------
 * forward declaration
 * ------------------------------------------------------------------------
 */
static uint32_t countRuns(ts_algo_list_t   *list,
                          ts_algo_compare_t compare);

static ts_algo_list_t *splitRuns(ts_algo_list_t   *list, 
                                 ts_algo_compare_t compare);

static ts_algo_rc_t mergeRuns(ts_algo_list_t   *ll,
                              ts_algo_compare_t compare);

/* ------------------------------------------------------------------------
 * sort
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_sort(ts_algo_list_t   *list,
                                  ts_algo_compare_t compare)
{
	uint32_t runs;
	ts_algo_list_t *x,*r;

	/* count runs */
	runs = countRuns(list,compare);
	if (runs == 0) return NULL;
	if (runs == 1) return ts_algo_list_copy(list);
	
	/* when there are more runs than half of the number
	 * of entries in the lists, we better sort the reverse
	 * of the input list */
	if (runs > list->len/2 + 1) {
		x = ts_algo_list_reverse(list);
		r = ts_algo_list_sort(x,compare);
		ts_algo_list_destroy(x); free(x);
		return r;
	}
	
	/* actually split the list into runs */
	x = splitRuns(list,compare);
	if (x == NULL) return NULL;

	/* merge the runs */
	if (mergeRuns(x,compare) != TS_ALGO_OK ||
	    x->len != 1 || x->head == NULL) 
	{
		ts_algo_list_destroyAll(x); free(x);
		return NULL;
	}
	/* the result is the only element of runs */
	r = x->head->cont;
	ts_algo_list_destroy(x); free(x);
	return r;
}

/* ------------------------------------------------------------------------
 * merge two (sorted) lists into a new (sorted) list
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_list_merge(ts_algo_list_t   *l1,
                                   ts_algo_list_t   *l2,
                                   ts_algo_compare_t compare)
{
	ts_algo_list_node_t *runner1,*runner2;
	ts_algo_list_t      *r;
	ts_algo_cmp_t        cmp;
	void                *cont;

	r = malloc(sizeof(ts_algo_list_t));
	if (r == NULL) return NULL;
	ts_algo_list_init(r);

	runner1=l1->head;runner2=l2->head;
	while(runner1 != NULL || runner2 != NULL) {
		/* when one of the lists is exhausted,
		 * we continue with the other, otherwise,
		 * we compare the two */
		if      (runner1==NULL) cmp = ts_algo_cmp_greater;
		else if (runner2==NULL) cmp = ts_algo_cmp_less;
		else cmp = compare(runner1->cont,runner2->cont);

		/* we remember the smaller one and go forward
		 * in that list */
		if  (cmp == ts_algo_cmp_greater) {
			cont = runner2->cont;
			runner2=runner2->nxt;
		} else {
			cont = runner1->cont;
			runner1=runner1->nxt;
		}
		if (ts_algo_list_append(r,cont) != TS_ALGO_OK) {
			ts_algo_list_destroy(r); free(r);
			return NULL;
		}
	}
	return r;
}

/* ------------------------------------------------------------------------
 * count runs
 * ------------------------------------------------------------------------
 */
static uint32_t countRuns(ts_algo_list_t   *list,
                          ts_algo_compare_t compare)
{
	uint32_t             runs=1;
	ts_algo_list_node_t *runner,*tmp;
	ts_algo_cmp_t cmp;
	
	runner=list->head;tmp=runner;
	runner=runner->nxt;
	while (runner != NULL) {
		cmp = compare(tmp->cont,runner->cont);
		/* this is a stepdown */
		if (cmp == ts_algo_cmp_greater) runs++; 
		tmp=runner;runner=runner->nxt;
	}
	return runs;
} 

/* ------------------------------------------------------------------------
 * split list into runs
 * ------------------------------------------------------------------------
 */
static ts_algo_list_t *splitRuns(ts_algo_list_t   *list, 
                                 ts_algo_compare_t compare)
{
	ts_algo_list_t *runs; /* the list of runs */
	ts_algo_list_t *run;  /* one run (runs is a list of lists) */
	ts_algo_list_node_t *runner,*tmp;
	ts_algo_cmp_t cmp;

	runs = malloc(sizeof(ts_algo_list_t));
	if (runs == NULL) return NULL;
	ts_algo_list_init(runs);

	run = malloc(sizeof(ts_algo_list_t));
	if (run == NULL) {
		free(runs); 
		return NULL;
	}
	ts_algo_list_init(run);

	runner=list->head;tmp=runner;
	runner=runner->nxt;
	while (runner != NULL) {
		/* the current one goes to the open run */
		if (ts_algo_list_append(run,tmp->cont) != TS_ALGO_OK)
		{
			ts_algo_list_destroy(run); free(run); 
			ts_algo_list_destroyAll(runs); free(runs);
			return NULL;
		}
		/* on a stepdown, we create a new run */
		cmp = compare(tmp->cont,runner->cont);
		if (cmp == ts_algo_cmp_greater) {
			if (ts_algo_list_append(runs,run) != TS_ALGO_OK) 
			{
				ts_algo_list_destroy(run); free(run); 
				ts_algo_list_destroyAll(runs); free(runs);
				return NULL;
			}
			run = malloc(sizeof(ts_algo_list_t));
			if (run == NULL) {
				ts_algo_list_destroyAll(runs); free(runs);
				return NULL;
			}
			ts_algo_list_init(run);
		}
		tmp=runner;runner=runner->nxt;
	}
	/* don't forget the last one */
	if (ts_algo_list_append(run,tmp->cont) != TS_ALGO_OK)
	{
		ts_algo_list_destroy(run); free(run); 
		ts_algo_list_destroyAll(runs); free(runs);
		return NULL;
	}
	if (ts_algo_list_append(runs,run) != TS_ALGO_OK) 
	{
		ts_algo_list_destroy(run); free(run); 
		ts_algo_list_destroyAll(runs); free(runs);
		return NULL;
	}
	return runs;
}

/* ------------------------------------------------------------------------
 * merge all the runs in the list recursively
 * until there is only one list left
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t mergeRuns(ts_algo_list_t   *ll,
                              ts_algo_compare_t compare)
{
	ts_algo_list_node_t *runner1,*runner2;
	ts_algo_list_t *r,*x;

	/* we are done */
	if (ll->len == 1) return TS_ALGO_OK;

	runner2=ll->head;runner1=runner2;
	runner2=runner2->nxt;
	while(runner2!=NULL) {
		r = ts_algo_list_merge(runner1->cont,runner2->cont,compare);
		if (r == NULL) return TS_ALGO_NO_MEM;

		/* cleaning up */
		ts_algo_list_remove(ll,runner1); 
		x=runner1->cont;
		ts_algo_list_destroy(x);free(x);free(runner1);
		x=runner2->cont;
		ts_algo_list_destroy(x);free(x);

		/* reuse second list */
		runner2->cont = r;
		runner2=runner2->nxt;

		/* all runs processed... */
		if (runner2 == NULL) break;

		/* ...or more to come */
		runner1=runner2;
		runner2=runner2->nxt;
	}
	/* merge the merged lists */
	return mergeRuns(ll,compare);
}
