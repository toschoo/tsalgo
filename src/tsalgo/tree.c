/* ========================================================================
 * (c) Tobias Schoofs, 2011 -- 2018
 * ========================================================================
 * AVL Tree
 * ========================================================================
 * Provides the AVL data structure 
 * with callbacks to handle application-defined data.
 * The implementation is based on 
 * Niklaus Wirth: "Algorithmen und Datenstrukturen", Stuttgart, 1983,
 *                p. 244-269.
 * ========================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include <tsalgo/tree.h>

/* duplicate detected */
#define DOUBLE 2

/* ------------------------------------------------------------------------
 * Allocate a new node making cont its content
 * ------------------------------------------------------------------------
 */
static ts_algo_tree_node_t *maketreenode(void *cont) {
	ts_algo_tree_node_t *t;

	if (cont == NULL) return NULL;
	t = malloc(sizeof(ts_algo_tree_node_t));
	if (t == NULL) return NULL;
	t->cont  = cont;
	t->bal   = 0;
	t->right = NULL; 
	t->left  = NULL; 
	return t;
}

/* ------------------------------------------------------------------------
 * Destroy a node. 
 * ------------------------------------------------------------------------
 */
static void destroytree(ts_algo_tree_t      *head,
                        ts_algo_tree_node_t *node) 
{
	if (node->left  != NULL) {
		destroytree(head,node->left); free(node->left);
	}
	if (node->right != NULL) {
		destroytree(head,node->right); free(node->right);
	}
	head->onDestroy(head,&node->cont);
}

/* ------------------------------------------------------------------------
 * Delete a node: call onDelete and free the node.
 * ------------------------------------------------------------------------
 */
static void deletenode(ts_algo_tree_t      *head,
                       ts_algo_tree_node_t *node) 
{
	head->onDelete(head,&node->cont); free(node);
}

/* ------------------------------------------------------------------------
 * Recursively search for a specific content in the tree.
 * The tree head "tree" is passed in to get access
 * to the comparison function.
 * ------------------------------------------------------------------------
 */
static void *treefind(ts_algo_tree_t      *tree,
                      ts_algo_tree_node_t *node,
                      void                *cont)
{
	int cmp = tree->compare(tree,cont,node->cont);
	if (cmp == ts_algo_cmp_equal) return node->cont;
	if (cmp == ts_algo_cmp_less) {
		if (node->left == NULL) return NULL;
		return treefind(tree,node->left,cont);
	} else {
		if (node->right == NULL) return NULL;
		return treefind(tree,node->right,cont);
	}
}

/* ------------------------------------------------------------------------
 * Compute the height of a subtree
 * ------------------------------------------------------------------------
 */
int treeheight(ts_algo_tree_node_t *node) {
	int hl,hr,h;
	if (node == NULL) return 0;
	hl = treeheight(node->left);
	hr = treeheight(node->right);
	if (hl > hr) h = hl; else h = hr;
	return h+1;
}

/* ------------------------------------------------------------------------
 * Check if this node is balanced
 * ------------------------------------------------------------------------
 */
static ts_algo_bool_t treebalanced (ts_algo_tree_node_t *node) {
	int hl,hr,d;
	if (node == NULL) return TRUE;
	hl = treeheight(node->left);
	hr = treeheight(node->right);
	if (hl > hr) d = hl-hr; else d = hr-hl;
	return (d<2);
}

/* ------------------------------------------------------------------------
 * Check for every node in the tree that
 * bal = height(right) - height(left)
 * ------------------------------------------------------------------------
 */
static ts_algo_bool_t baltest (ts_algo_tree_t      *tree,
                               ts_algo_tree_node_t *node) {
	int hl,hr;
	if (node == NULL) return TRUE;
	hl = treeheight(node->left);
	hr = treeheight(node->right);
	if (node->bal != hr-hl) return FALSE;
	if (!baltest(tree, node->left)) return FALSE;
	if (!baltest(tree, node->right)) return FALSE;
	return TRUE;
}

/* ------------------------------------------------------------------------
 * Predeclaration of insert (which is used in insertLeft and insertRight)
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t insert(ts_algo_tree_t      *tree,
                           ts_algo_tree_node_t *mom, 
                           ts_algo_tree_node_t *node, 
                           void                *cont, 
                           ts_algo_bool_t      *height);

/* ------------------------------------------------------------------------
 * Recursively insert a node into the left kid
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t insertLeft(ts_algo_tree_t      *tree,
                               ts_algo_tree_node_t *mom,
                               ts_algo_tree_node_t *node,
                               void                *cont,
                               ts_algo_bool_t      *height)
{
	if (node->left == NULL) {
		node->left = maketreenode(cont);
		if (node->left == NULL) return TS_ALGO_ERR;
		*height = TRUE;
		return TS_ALGO_OK;
	} else {
		return insert(tree,node,node->left,cont,height);
	}
}

/* ------------------------------------------------------------------------
 * Recursively insert a node into the right kid
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t insertRight (ts_algo_tree_t      *tree,
                                 ts_algo_tree_node_t *mom,
                                 ts_algo_tree_node_t *node,
                                 void                *cont,
                                 ts_algo_bool_t      *height)
{
	if (node->right == NULL) {
		node->right = maketreenode(cont);
		if (node->right == NULL) return TS_ALGO_ERR;
		*height = TRUE;
		return TS_ALGO_OK;
	} else {
		return insert(tree,node,node->right,cont,height);
	}
}

/* ------------------------------------------------------------------------
 * Big rotation from the left to the right.
 * ------------------------------------------------------------------------
 */
static void rotateLR(ts_algo_tree_node_t  *mom, 
                     ts_algo_tree_node_t *node,
                     ts_algo_bool_t   oninsert)
{
	ts_algo_tree_node_t *tmp1,*tmp2;

	if (node->left->right == NULL) return;

	if (mom->left == node) {
		mom->left  = node->left->right;
	} else {
		mom->right = node->left->right;
	}

	/* Rebalancing */
	if (oninsert) {
		if (node->left->right->bal == 1) {
			node->bal =  0;
			node->left->bal = -1;
		} else if (node->left->right->bal == -1) {
			node->bal =  1;
			node->left->bal = 0;
		} else {
			node->bal =  0;
			node->left->bal = 0;
		}
	} else {
		if (node->left->right->bal == -1) {
			node->bal = 1;
		} else {
			node->bal = 0;
		}
		if (node->left->right->bal == 1) {
			node->left->bal = -1;
		} else {
			node->left->bal = 0;
		}
	}

	node->left->right->bal = 0;
	
	tmp1 = node->left->right->right;
	tmp2 = node->left->right->left;

	node->left->right->right = node;
	node->left->right->left  = node->left;

	node->left->right = tmp2;
	node->left = tmp1;
}

/* ------------------------------------------------------------------------
 * Simple rotation from the left to the right.
 * ------------------------------------------------------------------------
 */
static void rotateRight(ts_algo_tree_node_t  *mom, 
                        ts_algo_tree_node_t *node,
                        ts_algo_bool_t    *height,
                        ts_algo_bool_t   oninsert) 
{
	ts_algo_tree_node_t *tmp;

	if (mom  == NULL) return;
	if (node == NULL) return;
	if (node->left == NULL) return;

	if (node->left->bal > 0) rotateLR(mom,node,oninsert);
	else {
		char b = node->left->bal;

		if (mom->left == node) 
			mom->left  = node->left;
		else 	mom->right = node->left;
	
		if (oninsert) node->left->bal = 0;
		else {
			if (b == 0) {
				node->left->bal = 1;
			} else {
				node->left->bal = 0;
			}
		}

		tmp = node->left->right;
		node->left->right = node;
		node->left = tmp;

		if (oninsert) node->bal = 0;
		else {
			if (b == 0) {
				node->bal = -1; *height = FALSE;
			} else {
				node->bal = 0;
			}
		}
	}
}

/* ------------------------------------------------------------------------
 * Big rotation from the right to the left.
 * ------------------------------------------------------------------------
 */
static void rotateRL(ts_algo_tree_node_t  *mom, 
                     ts_algo_tree_node_t *node,
                     ts_algo_bool_t   oninsert) 
{
	ts_algo_tree_node_t *tmp1,*tmp2;

	if (node->right->left == NULL) return;

	if (mom->left == node) 
		mom->left  = node->right->left;
	else 	mom->right = node->right->left;

	/* Rebalancing */
	if (oninsert) {
		if (node->right->left->bal == -1) {
			node->bal = 0;
			node->right->bal =  1;
		} else if (node->right->left->bal == 1) {
			node->bal =  -1;
			node->right->bal =  0;
		} else {
			node->bal = 0;
			node->right->bal =  0;
		}
	} else {
		if (node->right->left->bal == 1) {
			node->bal = -1;
		} else {
			node->bal = 0;
		} 
		if (node->right->left->bal == -1) {
			node->right->bal = 1;
		} else {
			node->right->bal = 0;
		}
	}

	node->right->left->bal = 0;
	
	tmp1 = node->right->left->left;
	tmp2 = node->right->left->right;

	node->right->left->left  = node;
	node->right->left->right = node->right;

	node->right->left = tmp2;
	node->right = tmp1;
}

/* ------------------------------------------------------------------------
 * Simple rotation from the right to the left.
 * ------------------------------------------------------------------------
 */
static void rotateLeft (ts_algo_tree_node_t  *mom, 
                        ts_algo_tree_node_t *node,
                        ts_algo_bool_t    *height,
                        ts_algo_bool_t   oninsert)
{
	ts_algo_tree_node_t *tmp;

	if (mom  == NULL) return;
	if (node == NULL) return;
	if (node->right == NULL) return;

	if (node->right->bal < 0) rotateRL(mom,node,oninsert);
	else {
		char b = node->right->bal;

		if (mom->left == node) 
			mom->left  = node->right;
		else 	mom->right = node->right;
	
		if (oninsert) node->right->bal = 0;
		else {
			if (b == 0) {
				node->right->bal = -1;
			} else {
				node->right->bal = 0;
			}
		}

		tmp = node->right->left;
		node->right->left = node;
		node->right = tmp;
		
		if (oninsert) node->bal = 0;
		else {
			if (b == 0) {
				node->bal = 1; *height = FALSE;
			} else {
				node->bal = 0;
			}
		}
	}
}

/* ------------------------------------------------------------------------
 * Recursively insert a new node.
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t insert(ts_algo_tree_t      *tree,
                           ts_algo_tree_node_t *mom,
                           ts_algo_tree_node_t *node, 
                           void                *cont,
                           ts_algo_bool_t      *height) 
{
	ts_algo_rc_t cmp = tree->compare(tree,cont,node->cont);
	ts_algo_rc_t rc;

	if (cmp == ts_algo_cmp_less) {
		rc = insertLeft(tree,mom,node,cont,height);
		if (rc != TS_ALGO_OK) return rc;
		if (*height) {
			if (node->bal < 0) {
				rotateRight(mom,node,height,TRUE);
				*height = FALSE;
			} else {
				node->bal--; 
				if (node->bal == 0) *height = FALSE;
			}
		}
		return rc;
	} else if (cmp == ts_algo_cmp_greater) {
		rc = insertRight(tree,mom,node,cont,height);
		if (rc != TS_ALGO_OK) return rc;
		if (*height) {
			if (node->bal > 0) {
				rotateLeft(mom,node,height,TRUE);
				*height = FALSE;
			} else {
				node->bal++;
				if (node->bal == 0) *height = FALSE;
			}
		}
		return rc;
	} else {
		*height = FALSE;
		rc = tree->onUpdate(tree,node->cont,cont);
		if (rc != TS_ALGO_OK) return rc;
		return DOUBLE; 
	}
}

/* ------------------------------------------------------------------------
 * Replace the node to be removed by the greatest node 
 * in the left subtree of that node.
 * ------------------------------------------------------------------------
 */
static void del(ts_algo_tree_t      *tree,
                ts_algo_tree_node_t *node,
                ts_algo_tree_node_t *mom,
                ts_algo_tree_node_t *runner,
                ts_algo_bool_t      *height)
{
	/* we have no yet found the greatest */
	if (runner->right != NULL) {
		char bal = runner->bal;
		del(tree,node,runner,runner->right,height);
		if (*height) {
			if (bal < 0) {
				rotateRight(mom,runner,height,FALSE);
			} else if (bal == 0) {
				runner->bal = -1; *height = FALSE;
			} else {
				runner->bal = 0;
			}
		}

	/* we have found it */
	} else {
		void *cont = node->cont;
		ts_algo_tree_node_t *tmp = runner;

		node->cont = runner->cont;
		runner->cont = cont;

		/* the greatest is child
		 * of the node we are removing */
		if (mom == node) {
			mom->left = tmp->left;
		} else {
			mom->right = tmp->left;
		}
		deletenode(tree, runner); 
		*height = TRUE;
	}
}

/* ------------------------------------------------------------------------
 * Recursively delete a node.
 * ------------------------------------------------------------------------
 */
static ts_algo_bool_t delete(ts_algo_tree_t      *tree,
                             ts_algo_tree_node_t *mom, 
                             ts_algo_tree_node_t *node, 
                             void                *cont, 
                             ts_algo_bool_t      *height)
{
	int cmp = tree->compare(tree,cont,node->cont);
	ts_algo_bool_t d;

	/* the node is less than the current node */
	if (cmp == ts_algo_cmp_less) {
		if (node->left == NULL) return FALSE;
		d = delete(tree,node,node->left,cont,height);
		if (*height) {
			if (node->bal > 0) {
				rotateLeft(mom,node,height,FALSE);
			} else if (node->bal == 0) {
				node->bal = 1; *height = FALSE;
			} else {
				node->bal = 0;
			}
		}
		return d;

	/* the node is greater than the current node */
	} else if (cmp == ts_algo_cmp_greater) {
		if (node->right == NULL) return FALSE;
		d = delete(tree,node,node->right,cont,height);
		if (*height) {
			if (node->bal < 0) {
				rotateRight(mom,node,height,FALSE);
			} else if (node->bal == 0) {
				node->bal = -1; *height = FALSE;
			} else {
				node->bal = 0;
			}
		}
		return d;

	/* we found it */
	} else {
		/* a leaf node */
		if (node->left == NULL && node->right == NULL) {
			if (mom->left == node) {
				mom->left = NULL;
			} else {
				mom->right = NULL;
			}
			deletenode(tree, node);
			*height = TRUE;

		/* node has left kid only */
		} else if (node->right == NULL) {
			if (mom->left == node) {
				mom->left = node->left;
			} else {
				mom->right = node->left;
			}
			deletenode(tree, node); 
			*height = TRUE;

		/* node has right kid only */
		} else if (node->left == NULL) {
			if (mom->left == node) {
				mom->left = node->right;
			} else {
				mom->right = node->right;
			}
			deletenode(tree, node); 
			*height = TRUE;

		/* node has two kids */
		} else {
			del(tree,node,node,node->left,height);
			if (*height) {
				if (node->bal > 0) {
					rotateLeft(mom,node,height,FALSE);
				} else if (node->bal == 0) {
					node->bal = 1; *height = FALSE;
				} else {
					node->bal = 0;
				}
			}
		}
		return TRUE;
	}
}

/* ------------------------------------------------------------------------
 * Tree to list
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t toList(ts_algo_tree_node_t *tree,
                           ts_algo_list_t      *list)
{
	if (tree->left != NULL) {
		if (toList(tree->left,list) != TS_ALGO_OK)
			return TS_ALGO_NO_MEM;
	}
	if (ts_algo_list_append(list,tree->cont) != TS_ALGO_OK)
		return TS_ALGO_NO_MEM;

	if (tree->right != NULL) {
		if (toList(tree->right,list) != TS_ALGO_OK)
			return TS_ALGO_NO_MEM;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Recursively search according to 'filter'.
 * Return first occurrence.
 * The tree head "tree" is passed in to get access
 * to the rsc.
 * ------------------------------------------------------------------------
 */
static void *treesearch(ts_algo_tree_t       *tree,
                        ts_algo_tree_node_t  *node,
                        const void        *pattern,
                        ts_algo_filter_t    filter)
{
	if (filter(tree,pattern,node->cont)) return node->cont;
	if (node->left != NULL) {
		void *rc = treesearch(tree, node->left, pattern, filter);
		if (rc != NULL) return rc;
	}
	if (node->right != NULL) {
		void *rc = treesearch(tree, node->right, pattern, filter);
		if (rc != NULL) return rc;
	}
	return NULL;
}

/* ------------------------------------------------------------------------
 * Recursively search according to 'filter'.
 * Return all occurrences.
 * The tree head "tree" is passed in to get access
 * to the rsc.
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t treefilter(ts_algo_tree_t      *tree,
                               ts_algo_list_t      *list,
                               ts_algo_tree_node_t *node,
                               const void       *pattern,
                               ts_algo_filter_t   filter)
{
	ts_algo_rc_t rc;

	if (filter(tree,pattern,node->cont)) {
		rc = ts_algo_list_append(list, node->cont);
		if (rc != TS_ALGO_OK) return rc;
	}
	if (node->left != NULL) {
		rc = treefilter(tree, list, node->left, pattern, filter);
		if (rc != TS_ALGO_OK) return rc;
	}
	if (node->right != NULL) {
		rc = treefilter(tree, list, node->right, pattern, filter);
		if (rc != TS_ALGO_OK) return rc;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Show the left kid, show the node and show the right kid.
 * ------------------------------------------------------------------------
 */
static void show(ts_algo_tree_t      *tree,
                 ts_algo_tree_node_t *node) 
{
	if (node->left != NULL) show(tree,node->left);
	tree->show(node->cont);
	if (node->right != NULL) show(tree,node->right);
}

/* ------------------------------------------------------------------------
 * Show the balance of the left kid,
 *      that of the node and 
 *      that of the right kid.
 * ------------------------------------------------------------------------
 */
static void showbal(ts_algo_tree_t *tree,
                    ts_algo_tree_node_t *node) 
{
	if (node->left != NULL) showbal(tree,node->left);
	tree->show(node->cont);
	fprintf(stdout, "\t%+d\n", node->bal);
	if (node->right != NULL) showbal(tree,node->right);
}

/* ------------------------------------------------------------------------
 * Allocate and initialise a new tree.
 * ------------------------------------------------------------------------
 */
ts_algo_tree_t *ts_algo_tree_new(ts_algo_comprsc_t compare,
                                 ts_algo_show_t    show,
                                 ts_algo_update_t  onUpdate,
                                 ts_algo_delete_t  onDelete,
                                 ts_algo_delete_t  onDestroy)
{
	ts_algo_tree_t *h;
	h = malloc(sizeof(ts_algo_tree_t));
	if (h == NULL) return NULL;
	if (ts_algo_tree_init(h,compare,show,onUpdate,
	                                     onDelete,
	                                     onDestroy) != TS_ALGO_OK) 
	{
		free(h); return NULL;
	}
	return h;
}

/* ------------------------------------------------------------------------
 * Initialise an already allocated tree.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_init(ts_algo_tree_t   *t, 
                               ts_algo_comprsc_t compare,
                               ts_algo_show_t    show,
                               ts_algo_update_t  onUpdate,
                               ts_algo_delete_t  onDelete,
                               ts_algo_delete_t  onDestroy)
{
	t->tree      = NULL;
	t->rsc       = NULL;
	t->count     = 0;
	t->compare   = compare;
	t->show      = show;
	t->onUpdate  = onUpdate;
	t->onDelete  = onDelete;
	t->onDestroy = onDestroy;
	t->dummy     = malloc(sizeof(ts_algo_tree_node_t));
	if (t->dummy == NULL) return TS_ALGO_ERR;
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Destroy the whole tree
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_destroy(ts_algo_tree_t *head) 
{
	if (head->tree != NULL) {
		destroytree(head,head->tree); free(head->tree);
		head->tree = NULL;
	}
	if (head->dummy != NULL) {
		free(head->dummy); head->dummy = NULL;
	}
}

/* ------------------------------------------------------------------------
 * Insert a new node
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_insert(ts_algo_tree_t *head, 
                                 void           *cont)
{
	ts_algo_bool_t h = FALSE;
	ts_algo_rc_t  rc;

	if (head->tree == NULL) {
		head->tree = maketreenode(cont);
		if (head->tree == NULL) return TS_ALGO_ERR;
		head->dummy->left = head->tree;
		head->count = 1;
		return TS_ALGO_OK;
	}
	rc = insert(head,head->dummy,head->tree,cont,&h);
	if (rc != TS_ALGO_OK) {
		if (rc == TS_ALGO_ERR) return TS_ALGO_ERR;
	} else {
		head->count++;
	}
	if (head->dummy->left != head->tree) {
		head->tree = head->dummy->left;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Delete a node
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_delete(ts_algo_tree_t *head,
                         void           *cont)
{
	ts_algo_bool_t h = FALSE;

	if (head->tree == NULL) return;
	if (delete(head,head->dummy,head->tree,cont,&h)) head->count--;
	if (head->dummy->left != head->tree) head->tree = head->dummy->left;
}

/* ------------------------------------------------------------------------
 * Find a node in the tree
 * ------------------------------------------------------------------------
 */
void *ts_algo_tree_find(ts_algo_tree_t *head,
                        void           *cont) 
{
	if (head->tree == NULL) return NULL;
	return treefind(head,head->tree,cont);
}

/* ------------------------------------------------------------------------
 * Measure the height of the tree
 * ------------------------------------------------------------------------
 */
int ts_algo_tree_height(ts_algo_tree_t *head) {
	if (head->tree == NULL) return 0;
	return treeheight(head->tree);
}

/* ------------------------------------------------------------------------
 * Check if the tree is balabnced
 * ------------------------------------------------------------------------
 */
ts_algo_bool_t ts_algo_tree_balanced(ts_algo_tree_t *head) {
	if (head->tree == NULL) return TRUE;
	return treebalanced(head->tree);
}

/* ------------------------------------------------------------------------
 * Check if for all subtrees bal == height(right)-height(left)
 * ------------------------------------------------------------------------
 */
ts_algo_bool_t ts_algo_tree_baltest(ts_algo_tree_t *head) {
	if (head->tree == NULL) return TRUE;
	return baltest(head,head->tree);
}

/* ------------------------------------------------------------------------
 * Show the tree's content
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_show(ts_algo_tree_t *head) {
	if (head->tree == NULL) return;
	show(head,head->tree);
}

/* ------------------------------------------------------------------------
 * Show the tree's balance
 * ------------------------------------------------------------------------
 */
void ts_algo_tree_showbal(ts_algo_tree_t *head) {
	if (head->tree == NULL) return;
	showbal(head,head->tree);
}

/* ------------------------------------------------------------------------
 * Tree to list
 * ------------------------------------------------------------------------
 */
ts_algo_list_t *ts_algo_tree_toList(ts_algo_tree_t *head) {
	ts_algo_list_t *list;

	if (head->tree == NULL) return NULL;

	list = malloc(sizeof(ts_algo_list_t));
	if (list == NULL) return NULL;
	ts_algo_list_init(list);

	if (toList(head->tree,list) != TS_ALGO_OK) {
		ts_algo_list_destroy(list); free(list);
		return NULL;
	}
	return list;
}

/* ------------------------------------------------------------------------
 * Recursively grab a generation
 * ------------------------------------------------------------------------
 */
static ts_algo_rc_t grab(ts_algo_tree_node_t *node,
                         ts_algo_list_t      *list,
                         int                   gen)
{
	/* final generation: get it */
	if (gen == 0) {
		if (node == NULL) {
			if (ts_algo_list_append(list, NULL) != TS_ALGO_OK)
				return TS_ALGO_ERR;
		} else {
			if (ts_algo_list_append(list, node->cont) != TS_ALGO_OK)
				return TS_ALGO_ERR;
		}
		return TS_ALGO_OK;
	}
	/* fill in NULL nodes for empty nodes */
	if (node == NULL) {
		if (ts_algo_list_append(list, NULL) != TS_ALGO_OK) {
			return TS_ALGO_ERR;
		}
		grab(NULL, list, gen-1);
		return TS_ALGO_OK;
	}
	/* grab next generation */
	if (grab(node->left, list, gen-1) != TS_ALGO_OK) return TS_ALGO_ERR;
	if (grab(node->right, list, gen-1) != TS_ALGO_OK) return TS_ALGO_ERR;

	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Grab one Generation
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_grabGeneration(ts_algo_tree_t *tree,
                                         ts_algo_list_t *list, int gen) 
{
	if (list == NULL) return TS_ALGO_ERR;
	if (tree == NULL) return TS_ALGO_ERR;
	if (gen < 0) return TS_ALGO_ERR;
	if (tree->tree == NULL) return TS_ALGO_OK;

	if (gen > 0) return grab(tree->tree, list, gen);

	if (ts_algo_list_append(list, tree->tree->cont) != TS_ALGO_OK) {
		return TS_ALGO_ERR;
	}
	return TS_ALGO_OK;
}

/* ------------------------------------------------------------------------
 * Search
 * ------------------------------------------------------------------------
 */
void *ts_algo_tree_search(ts_algo_tree_t *tree,
                          const void  *pattern,
                       ts_algo_filter_t filter) {
	if (tree == NULL) return NULL;
	if (tree->tree == NULL) return NULL;
	return treesearch(tree, tree->tree, pattern, filter);
}

/* ------------------------------------------------------------------------
 * Filter
 * ------
 * Filter traverses the tree and appends all nodes that complies with
 * the condition expressed in 'compare' to the list.
 * ------------------------------------------------------------------------
 */
ts_algo_rc_t ts_algo_tree_filter(ts_algo_tree_t    *tree,
                                 ts_algo_list_t    *list,
                                 void const     *pattern,
                                 ts_algo_filter_t filter) 
{
	if (tree == NULL) return TS_ALGO_OK;
	if (tree->tree == NULL) return TS_ALGO_OK;
	if (list == NULL) return TS_ALGO_INVALID;
	return treefilter(tree, list, tree->tree, pattern, filter);
}


