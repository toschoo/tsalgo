/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2016
 * ========================================================================
 * Base Types
 * ========================================================================
 */
#ifndef ts_algo_types_decl
#define ts_algo_types_decl

#include <stdint.h>

typedef char     ts_algo_byte_t;
typedef char     ts_algo_rc_t;
typedef char     ts_algo_bool_t;

#define TS_ALGO_KILO 1024
#define TS_ALGO_PAGE 4096
#define TS_ALGO_MEGA 1048576
#define TS_ALGO_GIGA 1073741824

#define FALSE 0 
#define TRUE 1

#define TS_ALGO_NOKEY 0

#define TS_ALGO_OK       0
#define TS_ALGO_ERR     -1
#define TS_ALGO_NO_MEM  -2
#define TS_ALGO_INVALID -3
#define TS_ALGO_FOPEN   -4
#define TS_ALGO_FREAD   -5
#define TS_ALGO_FWRITE  -6
#define TS_ALGO_FSEEK   -7
#define TS_ALGO_RENAME  -8
#define TS_ALGO_FFLUSH  -9

#define MAXPATH 4096

/* ------------------------------------------------------------------------
 * Order: equal, less or greater
 * ------------------------------------------------------------------------
 */
typedef enum {
	ts_algo_cmp_equal   = 0, /* the nodes are equal                */
	ts_algo_cmp_less    = 1, /* the left is less than the right    */
	ts_algo_cmp_greater = 2  /* the left is greater than the right */
} ts_algo_cmp_t;

/* ------------------------------------------------------------------------
 * Comparison function type
 * ------------------------
 * This is a callback for comparison operations.
 * The comparison follows the following convention:
 *
 * - if the left value is less than the right value
 *      the function shall return ts_algo_cmp_less
 *
 * - if the left value is greater than the right value
 *      the function shall return ts_algo_cmp_greater
 *
 * - if left and right value are equal
 *      the function shall return ts_algo_cmp_equal
 * ------------------------------------------------------------------------
 */
typedef ts_algo_cmp_t (*ts_algo_compare_t)(void*,void*);


/* ------------------------------------------------------------------------
 * Comparison with additional resource
 * -----------------------------------
 * This is a callback for comparison operations
 * with an additional resource.
 * The first pointer represents the additional resource.
 * The second and third pointer represent
 * the first and second element to be compared respectively.
 * ------------------------------------------------------------------------
 */
typedef ts_algo_cmp_t (*ts_algo_comprsc_t)(void*,void*,void*);

/* ------------------------------------------------------------------------
 * Show callback
 * ------------------------------------------------------------------------
 */
typedef void (*ts_algo_show_t)(void*);

/* ------------------------------------------------------------------------
 * event callback
 * ------------------------------------------------------------------------
 */
typedef ts_algo_rc_t  (*ts_algo_update_t)(void*,void*,void*);
typedef void          (*ts_algo_delete_t)(void*,void**);

#endif
