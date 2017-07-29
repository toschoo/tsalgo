/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2017
 * ========================================================================
 * AVL Tree
 * ========================================================================
 * Provides the AVL data structure 
 * with callbacks to handle application-defined data.
 * ========================================================================
 */
#ifndef ts_algo_tree_decl
#define ts_algo_tree_decl

#include <tsalgo/types.h>
#include <tsalgo/list.h>

/* ------------------------------------------------------------------------
 * A node in a tree
 * ------------------------------------------------------------------------
 */
typedef struct ts_algo_tree_node_st {
	void                        *cont;  /* the content    */
	char                         bal;   /* balancing flag */
	struct ts_algo_tree_node_st *right; /* right kid      */
	struct ts_algo_tree_node_st *left;  /* left kid       */
} ts_algo_tree_node_t; 

/* ------------------------------------------------------------------------
 * Head node of a tree
 * ------------------------------------------------------------------------
 */
typedef struct {
	ts_algo_tree_node_t    *tree;  /* the first node           */
	ts_algo_tree_node_t   *dummy;  /* mom of the first node    */
	uint32_t               count;  /* how many nodes are there */
	ts_algo_compare_t    compare;  /* comparison method        */
	ts_algo_show_t          show;  /* show method              */
	ts_algo_update_t    onUpdate;  /* on update                */
	ts_algo_delete_t    onDelete;  /* on delete                */
	ts_algo_delete_t   onDestroy;  /* on destroy               */
} ts_algo_tree_t;

/* ------------------------------------------------------------------------
 * Allocate a new tree and initialise it
 * Receives
 * - the comparison method for the intended content type
 * - the show       method for the intended content type
 * - the onUpdate   method for the intended content type
 * - the onDelete   method for the intended content type
 * - the onDestroy  method for the intended content type
 * 
 * Fails only if there was not enough memory 
 *               to create or initialise the tree.
 * ------------------------------------------------------------------------
 */
ts_algo_tree_t *ts_algo_tree_new(ts_algo_compare_t compare,
                                 ts_algo_show_t    show,
                                 ts_algo_update_t  onUpdate,
                                 ts_algo_delete_t  onDelete,
                                 ts_algo_delete_t  onDestroy);

/* ------------------------------------------------------------------------
 * Initialise an already allocated tree
 * Receives
 * - the comparison method for the intended content type
 * - the show       method for the intended content type
 * - the onUpdate   method for the intended content type
 * - the onDelete   method for the intended content type
 * - the onDestroy  method for the intended content type
 * 
 * Fails only if there was not enough memory
 *               to initialise the tree.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_init(ts_algo_tree_t   *tree,
                               ts_algo_compare_t compare,
                               ts_algo_show_t    show,
                               ts_algo_update_t  onUpdate,
                               ts_algo_delete_t  onDelete,
                               ts_algo_delete_t  onDestroy);

/* ------------------------------------------------------------------------
 * Destroy everything within a tree.
 * On each node, the onDestroy method is called,
 * before the node itself is freed.
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_destroy(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Insert a new node
 *
 * The position of the new node is determined by the compare method.
 * If the key does already exists, onUpdate is called.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_insert(ts_algo_tree_t *tree, 
				 void           *cont);

/* ------------------------------------------------------------------------
 * Find a node in the tree using the compare method.
 * If the node exists, it is returned. 
 * Otherwise, NULL is returned.
 * ------------------------------------------------------------------------
 */
void *ts_algo_tree_find(ts_algo_tree_t *tree,
                        void           *node);

/* ------------------------------------------------------------------------
 * Delete a node
 *
 * Delete a node using the compare method for finding the node.
 * Before the node is removed from the tree, the onDelete method is called.
 * If the node is not in the tree, delete has no effect.
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_delete(ts_algo_tree_t *tree,
                         void           *node);

/* ------------------------------------------------------------------------
 * Measure the height of the tree
 * ------------------------------------------------------------------------
 */
int ts_algo_tree_height(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Check if the tree is balanced
 * ------------------------------------------------------------------------
 */
ts_algo_bool_t ts_algo_tree_balanced(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Check for every node in the tree that
 * bal = height(right) - height(left).
 * ------------------------------------------------------------------------
 */
ts_algo_bool_t ts_algo_tree_baltest(ts_algo_tree_t *head);

/* ------------------------------------------------------------------------
 * Show the contents of the tree using the show method.
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_show(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Show the balance of the tree.
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_showbal(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Tree to list
 * ------------
 * The list is allocated and its memory is to be handled
 * by the application, i.e. it must be destroyed and freed.
 * Note however that the tree content is not copied; the list
 * contains pointers to the content in the tree.
 * The list, therefore, should not live longer than the tree
 * and the content in the list should not be destroyed.
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_tree_toList(ts_algo_tree_t *tree);

/* ------------------------------------------------------------------------
 * Grab one Generation
 * -------------------
 * The list must be allocated and initialised.
 * The tree content of each generation is stored in the list, but
 * it is not copied; the list contains pointers to the content in the tree.
 * The list, therefore, should not live longer than the tree
 * and the content in the list should not be destroyed.
 *
 * Empty nodes are represented by nodes with cont = NULL in the list.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_grabGeneration(ts_algo_tree_t *tree,
                                         ts_algo_list_t *list,
                                         int gen);

#endif
