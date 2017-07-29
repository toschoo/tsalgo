/* ========================================================================
 * (c) Tobias Schoofs, 2016
 * ========================================================================
 * Bufsort
 * ========================================================================
 * Provides functions to sort a buffer in memory, namely
 * - merge sort
 * - quick sort
 * ========================================================================
 */
#ifndef ts_algo_sort_buf_decl
#define ts_algo_sort_buf_decl

#include <tsalgo/types.h>

/* ------------------------------------------------------------------------
 * Buffer
 * ------
 *
 * Internally, the sort algorithms use buffers of bytes.
 * Whatever your type is, cast it to ts_algo_sort_buf_t, when
 * you pass it to the sorting function.
 * ------------------------------------------------------------------------
 */
typedef char *ts_algo_sort_buf_t;

/* ------------------------------------------------------------------------
 * MergeSort
 * ---------
 *
 * Sorts a buffer in memory into a freshly allocated buffer using mergesort.
 * The original buffer is never written!
 * Note that the return value is usually a freshly allocated buffer.
 * If the input was already sorted, however,
 * the return value is just a pointer to the input (src).
 * Therefore, release the return value using 
 *
 *                             ts_algo_sort_buf_release
 *
 * Do not release it using "free" directly.
 *
 * The parameters are
 * - src      : the input buffer
 * - size     : the size of the buffer in bytes (!)
 * - storesize: the size of one element in the buffer
 * - compare  : the comparison callback (see ts_algo_compare_t above)
 * ------------------------------------------------------------------------
 */
void *ts_algo_sort_buf_merge(const ts_algo_sort_buf_t src,
                             size_t                  size,
                             size_t             storesize,
                             ts_algo_compare_t   compare);

/* ------------------------------------------------------------------------
 * QuickSort
 * ---------
 *
 * Sorts a buffer in memory into a freshly allocated buffer using quicksort.
 * The original buffer is never written!
 * Note that the return value is usually a freshly allocated buffer.
 * If the input was already sorted, however,
 * the return value is just a pointer to the input (src).
 * Therefore, release the return value using 
 *
 *                             ts_algo_sort_buf_release
 *
 * Do not release it using "free" directly.
 *
 * The parameters are
 * - src      : the input buffer
 * - size     : the size of the buffer in bytes (!)
 * - storesize: the size of one element in the buffer
 * - compare  : the comparison callback (see ts_algo_compare_t above)
 * ------------------------------------------------------------------------
 */
void *ts_algo_sort_buf_quick(const ts_algo_sort_buf_t src,
                             size_t size,
                             size_t storesize,
                             ts_algo_compare_t compare);

/* ------------------------------------------------------------------------
 * Release buffer
 * --------------
 * 
 * Releases a buffer created by a sort function.
 * The parameters are:
 * - src: the original (unsorted) buffer you passed in 
 *        to ts_algo_sort_buf_merge
 * - trg: the return value of that function.
 *
 * If the two pointers are pointing to the same memory,
 * trg is not freed; otherwise it is freed.
 * ------------------------------------------------------------------------
 */
void ts_algo_sort_buf_release(ts_algo_sort_buf_t src, ts_algo_sort_buf_t trg);

#endif
