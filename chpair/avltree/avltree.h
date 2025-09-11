#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdbool.h>

/* Type definitions */

typedef int (*avl_compare_t)(const void*, const void*);

typedef struct avl_tree {
    struct avl_node* root; // Pointer to first node (init at NULL)
    avl_compare_t    comp; // Key three-way comparison function
} avl_tree_t;

/* Basic AVL tree API */

// Search value in tree by key and copy to out, return is found or not
bool avl_search(const avl_tree_t* tree, const void* key, void** out);
// Insert value in tree by key, if key exist replace old value
void avl_insert(/* */ avl_tree_t* tree, const void* key, void* data);
// Remove value by key from tree
void avl_remove(/* */ avl_tree_t* tree, const void* key);
// Release all nodes in tree
void avl_delete(/* */ avl_tree_t* tree);

/* Utility API */

// Call func for each nodes with arguments (key, data) in ascending key order
void avl_forall(const avl_tree_t* tree, void (*func)(const void*, void*));
// Print all tree to stdout with using putkey/putdata (optional)
void avl_output(const avl_tree_t* tree, void (*putkey)(const void*), void (*putdata)(void*));

#endif // AVL_TREE_H