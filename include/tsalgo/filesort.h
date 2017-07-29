/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * FileSort
 * ========================================================================
 * Provides functions for external sorting.
 * It is based on the tape sorting alorithms presented
 * in Knuth TAOCP, Volume 3, pp. 248ff.
 * ========================================================================
 */
#ifndef ts_algo_sort_file_decl
#define ts_algo_sort_file_decl

#include <tsalgo/types.h>

/* ------------------------------------------------------------------------
 * External sorting
 * ----------------
 *
 * Sorts a file that is too big to be loaded into memory
 *
 * parameters:
 * -----------
 * - fname    : name of the file to be sorted
 * - oname    : name of the sorted output file
 * - bufsize  : size of memory buffers to use in bytes
 * - filesize : size of the file in bytes
 * - storesize: size of things in the file to be sorted in bytes
 * - compare  : user-defined comparison function
 *
 * preconditions:
 * --------------
 * - storesize | bufsize
 * - bufsize   < filesize
 * - bufsize fits into memory
 *
 * result:
 * -------
 * a sorted file called <oname>
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_sort_file(char             *fname,
                               char             *oname,
                               size_t            bufsize,
                               size_t            filesize,
                               size_t            storesize,
                               ts_algo_compare_t compare);

/* ------------------------------------------------------------------------
 * Merge two sorted files into one
 *
 * parameters:
 * -----------
 * - f1name   : name of the first  (sorted) file 
 * - f2name   : name of the second (sorted) file 
 * - oname    : name of the sorted output file
 * - bufsize  : size of memory buffers to use in bytes
 * - filesize1: size of the first  file in bytes
 * - filesize2: size of the second file in bytes
 * - storesize: size of things in the file to be sorted in bytes
 * - compare  : user-defined comparison function
 *
 * preconditions:
 * --------------
 * - storesize | bufsize
 * - bufsize   < filesize (1 and 2)
 * - bufsize fits into memory
 *
 * result:
 * -------
 * - file consisting of all records in file 1 and file 2
 * - if both input files are sorted, the output is sorted too
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_merge_files(char             *f1name,
                                 char             *f2name,
                                 char             *oname,
                                 size_t            filesize1,
                                 size_t            filesize2,
                                 size_t            storesize,
                                 ts_algo_compare_t compare);
#endif
