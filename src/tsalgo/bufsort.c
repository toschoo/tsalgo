/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * Bufsort
 * -------
 *
 * "Have you ever heard of Rock'n'Roll?
 *  Study this book!"
 *
 * Aki Kaurismäki, Leningrad Cowboys.
 *
 * ========================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <tsalgo/bufsort.h>
#include <tsalgo/list.h>

/* ------------------------------------------------------------------------
 * A run is a sorted region within a buffer
 * -----
 * There is an exhaustive discussion of runs and stepdowns
 * in Volume 3 of TAOCP ("Searching and Sorting").
 * 
 * We interpret runs as pointers into the buffer.
 * - start points to the position where the first element starts (e.g. 0)
 * - end   points to the position *after* the last one
 * ------------------------------------------------------------------------
 */
typedef struct {
	size_t        start;
	size_t        end;
} ts_algo_sort_run_t;

/* ------------------------------------------------------------------------
 * Single runs are stored in a list.
 * Eventually, we destroy the list with all memory resources
 * related to the list.
 * ------------------------------------------------------------------------
 */
static void destroyRuns(ts_algo_list_t *runs) {
	ts_algo_list_node_t *runner;
	ts_algo_sort_run_t  *bl;

	runner = runs->head;
	while (runner != NULL) {
		bl = runner->cont;
		free(bl);
		runner = runner->nxt;
	}
	ts_algo_list_destroy(runs);
}

/* ------------------------------------------------------------------------
 * Get one single run
 * ------------------
 *
 * This is equivalent to
 * - adding all elements 
 * - until we find a stepdown
 *
 * Parameters:
 * - the buffer
 * - the size of the buffer (in bytes)
 * - the size of one element (in bytes)
 * - the comparison callback
 * ------------------------------------------------------------------------
 */
static size_t getRun(const ts_algo_sort_buf_t  buf, 
                           size_t         size, 
                           size_t         storesize, 
                           ts_algo_compare_t  compare)
{
	size_t i;

	for (i=0;i+storesize<size;i+=storesize) {
		/* the left one is greater than the right one:
		 * That is a stepdown! */
		if (compare(buf+i,buf+i+storesize) == ts_algo_cmp_greater)
			break;
	}
	/* we return the position *before* the last element */
	return i;
}

/* ------------------------------------------------------------------------
 * Getting all runs in a given buffer
 * ----------------------------------
 *
 * Parameters:
 * - the buffer
 * - the size of the buffer (in bytes)
 * - the size of one element (in bytes)
 * - the comparison callback
 * - the list of runs (an already initialised list)
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t getRuns(const ts_algo_sort_buf_t  buf,
                                  size_t         size,
                                  size_t         storesize, 
                                  ts_algo_compare_t  compare,
                                  ts_algo_list_t *runs)
{
	size_t idx;
	size_t i=0;
	ts_algo_sort_run_t *run;

	while (i<size) {
		idx=getRun(buf+i,size-i,storesize,compare);

		run = malloc(sizeof(ts_algo_sort_run_t));
		if (run == NULL) {
			return -1;
		}

		run->start=i;
		i+=idx+storesize; /* the position after the last element */
		run->end=i;
		
		/* add the run to the list of runs */
		if (ts_algo_list_append(runs,run) != 0) {
			return -1;
		}

	}
	return 0;
}

/* ------------------------------------------------------------------------
 * A debug function printing runs that turned out to be very usefull
 * ------------------------------------------------------------------------
 */
void ts_algo_sort_showRuns(ts_algo_list_t *runs) {
	ts_algo_list_node_t *runner;
	ts_algo_sort_run_t  *bl;

	printf("Runs: %u\n", runs->len);
	runner = runs->head;
	while (runner != NULL) {
		bl = runner->cont;
		printf("  run from %lu to %lu (length: %lu, %lu)\n", 
		       bl->start,bl->end,
		       bl->end-bl->start,
		       (bl->end-bl->start)/4);
		runner = runner->nxt;
	}
}

/* ------------------------------------------------------------------------
 * Reverse a buffer 
 * ----------------
 *
 * "So the last will be first, and the first will be last"
 * (Matthew 20:16)
 *
 * Parameters:
 * - the source buffer
 * - the target buffer
 * - the size of the buffers (in bytes)
 * - the size of one element (in bytes)
 * ------------------------------------------------------------------------
 */
static void reverse(const ts_algo_sort_buf_t src,
                          ts_algo_sort_buf_t trg,
                          size_t        size,
                          size_t        storesize)
{
	size_t i,j=size-storesize;

	for(i=0;i<size;i+=storesize) {
		memcpy(trg+i,src+j,storesize); j-=storesize;
	}
}

/* ------------------------------------------------------------------------
 * Merge two runs
 * --------------
 *
 * Merging works like this:
 * - we compare the head of one and the head of two
 * - if that of one is the smaller one,
 *      - we copy the head of one to the target
 *           and increment the index of one
 * - if that of two is the smaller one (or if they are equal)
 *      - we copy the head of two to the target
 *           and increment the index of two
 * - if one of the list is exhausted, 
 *      - we copy the rest of the other list
 *
 * Parameters:
 * - the first  run
 * - the second run
 * - the target buffer
 * - the size of the firts  run (in bytes)
 * - the size of the second run (in bytes)
 * - the size of one element (in bytes)
 * - the comparison function
 * ------------------------------------------------------------------------
 */
static size_t mergeRuns (const ts_algo_sort_buf_t    one,
                               ts_algo_sort_buf_t    two,
                               ts_algo_sort_buf_t    trg,
                               size_t              size1,
                               size_t              size2,
                               size_t          storesize,
                               ts_algo_compare_t compare)
{
	size_t i=0,j=0,z=0;

	while (i<size1 && j<size2) {
		if (compare(one+i,two+j) == ts_algo_cmp_less) {
			memcpy(trg+z,one+i,storesize);
			i+=storesize;
		} else {
			memcpy(trg+z,two+j,storesize);
			j+=storesize;
		}
		z+=storesize;
	}
	if (i<size1) {
		memcpy(trg+z,one+i,size1-i);z+=size1-i;
	} else if (j<size2) {
		memcpy(trg+z,two+j,size2-j);z+=size2-j;
	}
	return z;
}

/* ------------------------------------------------------------------------
 * Merge all runs in a buffer
 * --------------------------
 *
 * We merge pairs of consecutive runs;
 * the last run (if the number of runs is odd)
 * is just copied to the target.
 * On the way, we condense the list of runs.
 *
 * Parameters:
 * - the source buffer
 * - the target buffer
 * - the size of the buffers (in bytes)
 * - the size of one element (in bytes)
 * - the of runs
 * - the comparison function
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t mergeBuf(const ts_algo_sort_buf_t src,
                                   ts_algo_sort_buf_t trg,
                                   size_t size,
                                   size_t storesize,
                                   ts_algo_list_t *runs,
                                   ts_algo_compare_t compare)
{
	ts_algo_sort_buf_t b1,b2;
	ts_algo_list_node_t *runner,*tmp;
	ts_algo_sort_run_t *run1,*run2;
	size_t x,z;

	z=0;
	runner = runs->head;
	while(runner != NULL) {
		run1   = runner->cont;
		tmp    = runner; /* remember previous one */
		runner = runner->nxt;
		/* one is left over */
		if (runner == NULL) {
			x=run1->end -
                          run1->start;
			memcpy(trg+z,src+run1->start,x);
			run1->start=z;
			run1->end=run1->start+x;
			break;
		}
		/* the second of the pair */
		run2 = runner->cont;
		/* get the runs in the buffer */
		b1   = src+run1->start;
		b2   = src+run2->start;
		x=z; /* remember current target position */
		/* merge the pair of runs */
		z+=mergeRuns(b1,b2,trg+z,
                             run1->end-run1->start, /* size1 */
		             run2->end-run2->start, /* size2 */
		             storesize, compare);
		
		/* we can dispose one of the runs
		   and condense the information in the other node */
		ts_algo_list_remove(runs,tmp);
		run2->start=x; /* run2 now spans run1 and run2 */
		run2->end  =z;
		/* clean up */
		free(run1); free(tmp);
		runner = runner->nxt;
	}
	return 0;
}

/* ------------------------------------------------------------------------
 * Merge buffer until there is only one run left
 * ------------
 *
 * We merge all runs in a buffer pairwise.
 * If only one run is left, we are done.
 * Otherwise we iterate.
 *
 * Buffer handling:
 * - we have three buffers:
 *   1) the source
 *   2) helper 1
 *   3) helper 2
 *
 * - we merge source into helper 1
 * - on recursion, we make helper 1 the source
 * - and merge into helper 2
 * - source is not used anymore in recursion 
 *   (it may be the input buffer, which must not be written)
 *
 * NOTE: we use recursion here.
 *       Recursion is more costly than iteration,
 *       especially because we have many parameters.
 *       On the other -- and this is point of mergesort --
 *       we do it only log n times (e.g. ~10 times for 1000 elements).
 *       Optimisation may resolve recursion, 
 *       but the effect will be minimal. 
 * 
 * Parameters:
 * - the source buffer
 * - the first  helper buffer
 * - the second helper buffer
 * - the size of the buffers (in bytes)
 * - the size of one element (in bytes)
 * - the of runs
 * - the comparison function
 * ------------------------------------------------------------------------
 */
static ts_algo_sort_buf_t buffermerge (const ts_algo_sort_buf_t src,
                                        ts_algo_sort_buf_t hlp1,
                                        ts_algo_sort_buf_t hlp2,
                                        size_t size,
                                        size_t storesize,
                                        ts_algo_list_t *runs,
                                        ts_algo_compare_t compare)
{
	
	if (mergeBuf(src,hlp1,size,storesize,runs,compare) != 0)
	{
		free(hlp1); free(hlp2);
		destroyRuns(runs);
		return NULL;
	}

	if (runs->len == 1) {
		free(hlp2); /* we don't need this one anymore */
		destroyRuns(runs);
		return hlp1;
	}
	/* recursion exchanging helpers */
	return buffermerge(hlp1,hlp2,hlp1,size,storesize,runs,compare);
}

/* ------------------------------------------------------------------------
 * Quick Sort
 * ----------
 * Following the implementation by Jon Bentley,
 * described in 
 *
 * "The Most Beautiful Code I Never Wrote" in 
 * Andy Oram & Greg Wilson: "Beautiful Code".
 * ------------------------------------------------------------------------
 */
static size_t randidx(size_t storesize, size_t l, size_t u) {
	size_t n,t;
	/* get a random number in the range l to u */
	n = u-l+1;
	t=rand()%n;
	/* move into the current subbuffer */
	t+=l;
	/* round it to storesize */
	t /= storesize;
	t *= storesize;
	return t;
}

/* ------------------------------------------------------------------------
 * Swap two positions in the buffer
 * ------------------------------------------------------------------------
 */
static void swap(ts_algo_sort_buf_t x, 
                 ts_algo_sort_buf_t t,
                 size_t storesize,
                 size_t i, 
                 size_t j) 
{
	if (i==j) return;
	memcpy(t,x+i,storesize); 
	memcpy(x+i,x+j,storesize);  
	memcpy(x+j,t,storesize);
}

/* ------------------------------------------------------------------------
 * Jon Bentley's implementation of quicksort
 * ------------------------------------------------------------------------
 */
static void quicksort(ts_algo_sort_buf_t x, 
                      ts_algo_sort_buf_t hlp,
                      size_t storesize,
                      size_t l, size_t u,
                      ts_algo_compare_t compare) 
{
	size_t i, m;

	if (l>=u) return;

	/* select a pivot and a slot 
	 * where we start to store the smaller ones */
	swap(x,hlp,storesize,l,randidx(storesize,l,u));
	m = l;

	/* compare the (sub)buffer */
	for (i=l+storesize;i<=u;i+=storesize) {
		if (compare(x+i,x+l) == ts_algo_cmp_less) {
			m+=storesize;
			swap(x,hlp,storesize,m,i);
		}
	}
	/* swap l and m (the pivot is now 
	 * the greatest of the small ones) */
	swap(x,hlp,storesize,l,m);

	/* sort the result buffers */
	if (m!=0) { /* l+storesize == u, there are no smaller ones */
		quicksort(x,hlp,storesize,l,m-storesize,compare);
	}
	quicksort(x,hlp,storesize,m+storesize,u,compare);
}

/* ------------------------------------------------------------------------
 * Mergesort
 * ------------------------------------------------------------------------
 */
void *ts_algo_sort_buf_merge(const ts_algo_sort_buf_t src,
                              size_t        size,
                              size_t        storesize,
                              ts_algo_compare_t compare)
{
	ts_algo_list_t runs;
	size_t no;
	ts_algo_sort_buf_t hlp1,hlp2,hlp0=src;

	/* parameter validation */
	if (src == NULL)      return NULL;
	if (size == 0)        return NULL;
	if (storesize == 0)   return NULL;
	if (storesize > size) return NULL;

	/* the number of elements in the buffer */
	no = size/storesize-1;

	/* get runs */
	ts_algo_list_init(&runs);
	if (getRuns(src,size,storesize,compare,&runs) != 0) {
		destroyRuns(&runs);
		return NULL;
	}

	/* if we have only one run, there is nothing to do */
	if (runs.len == 1) {
		destroyRuns(&runs);
		return src;
	}
	hlp1 = malloc(size);
	if (hlp1 == NULL) {
		destroyRuns(&runs);
		return NULL;
	}

	/* if there are more runs than half of the number of elements,
	   reversing the buffer will reduce the number of runs */
	if (runs.len > no/2+1) {
		reverse(src,hlp1,size,storesize);

		/* recompute runs for reversed buffer */
		destroyRuns(&runs);
		ts_algo_list_init(&runs);
		hlp0=hlp1; /* we use hlp1 instead of src */
		if (getRuns(hlp0,size,storesize,compare,&runs) 
		    != 0)
		{
			destroyRuns(&runs);
			free(hlp1);
			return NULL;
		}
		if (runs.len == 1) {
			destroyRuns(&runs);
			return hlp0;
		}
	}
	hlp2 = malloc(size);
	if (hlp2 == NULL) {
		destroyRuns(&runs);
		free(hlp1);
		return NULL;
	}
	/* merge buffers, 
	 * note that buffermerge takes care of
	 * freeing the memory of the helper buffer and
	 * the list of runs.
	 */
	return buffermerge(hlp0,hlp2,hlp1,size,storesize,&runs,compare);
}

/* ------------------------------------------------------------------------
 * Quicksort
 * ------------------------------------------------------------------------
 */
void *ts_algo_sort_buf_quick(const ts_algo_sort_buf_t src,
                              size_t size,
                              size_t storesize,
                              ts_algo_compare_t compare)
{
	int i;
	ts_algo_bool_t sorted = TRUE;
	ts_algo_sort_buf_t hlp1,hlp2;

	/* parameter validation */
	if (src == NULL)      return NULL;
	if (size == 0)        return NULL;
	if (storesize == 0)   return NULL;
	if (storesize > size) return NULL;

	for(i=size-storesize;i!=0;i-=storesize) {
		if (compare(&src[i], &src[i-storesize]) == ts_algo_cmp_less) {
			sorted = FALSE; break;
		}
	}
	if (sorted) return src;

	/* the helper buffer */
	hlp1 = malloc(size);
	if (hlp1 == NULL) {
		return NULL;
	}
	memcpy(hlp1,src,size);

	/* this helper is just for swapping positions */
	hlp2 = malloc(storesize);
	if (hlp2 == NULL) {
		free(hlp1);
		return NULL;
	}
	srand((unsigned int)time(NULL));
	quicksort(hlp1,hlp2,storesize,0,size-storesize,compare);
	free(hlp2);
	return hlp1;
}

/* ------------------------------------------------------------------------
 * Release buffer
 * ------------------------------------------------------------------------
 */
void ts_algo_sort_buf_release(ts_algo_sort_buf_t src, ts_algo_sort_buf_t trg) 
{
	if (trg == NULL) return;
	if (src == trg)  return;
	free(trg);
}
