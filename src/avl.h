#ifndef AVL_H
#define AVL_H

#include <cstdint>
#include <stddef.h>
#include <stdint.h>

struct AVLNode {
    uint32_t depth = 0; // height of the tree
    uint32_t count = 0; // size of the tree, used by the avl_offset operation
    AVLNode *left = nullptr;
    AVLNode *right = nullptr;
    AVLNode *parent = nullptr;
};

void avl_init(AVLNode *node);

uint32_t avl_depth(AVLNode *node);
uint32_t avl_count(AVLNode *node);
uint32_t max(uint32_t lhs, uint32_t rhs);
void avl_update(AVLNode *node);

AVLNode *rotate_left(AVLNode *node);
AVLNode *rotate_right(AVLNode *node);

AVLNode *avl_fix_left(AVLNode *root);
AVLNode *avl_fix_right(AVLNode *root);

AVLNode *avl_rebalance(AVLNode *node);

AVLNode *avl_delete(AVLNode *node);

/**
 * Offset into the succeeding/preceding node
 * NOTE: worst-case time complexity is O(log(n)), regardless of how long the
 * offset is
 */
AVLNode *avl_offset(AVLNode *node, int64_t offset);

#endif /* AVL_H */
