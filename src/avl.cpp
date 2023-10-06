#include "avl.h"
#include <cstdint>

void avl_init(AVLNode *node) {
    node->depth = 1;
    node->count = 1;
    node->left = node->right = node->parent = nullptr;
}

uint32_t avl_depth(AVLNode *node) { return node ? node->depth : 0; }
uint32_t avl_count(AVLNode *node) { return node ? node->count : 0; }
uint32_t max(uint32_t lhs, uint32_t rhs) { return lhs < rhs ? rhs : lhs; }

void avl_update(AVLNode *node) {
    node->depth = 1 + max(avl_depth(node->left), avl_depth(node->right));
    node->count = 1 + avl_count(node->left) + avl_count(node->right);
}

AVLNode *rotate_left(AVLNode *node) {
    AVLNode *new_node = node->right;
    if (!new_node)
        return node;
    if (new_node->left) {
        new_node->left->parent = node;
    }
    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}

AVLNode *rotate_right(AVLNode *node) {
    AVLNode *new_node = node->left;
    if (!new_node)
        return node;
    if (new_node->right) {
        new_node->right->parent = node;
    }
    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}

/**
 * Left subtree is too deep
 */
AVLNode *avl_fix_left(AVLNode *root) {
    if (avl_depth(root->left->left) < avl_depth(root->left->right)) {
        root->left = rotate_left(root->left);
    }
    return rotate_right(root);
}

AVLNode *avl_fix_right(AVLNode *root) {
    if (avl_depth(root->right->right) < avl_depth(root->right->left)) {
        // right-left case
        /**
         *       b              b
         *      / \            / \
         *     a   c  ==>     a   d
         *        /                \
         *       d                  c
         */
        root->right = rotate_right(root->right);
    }

    /**
     *       d
     *      / \
     *     b   c
     *    /
     *   a
     */
    return rotate_left(root);
}

/**
 * Rebalance the AVL tree, starting from the affected node
 */
AVLNode *avl_rebalance(AVLNode *node) {
    while (true) {
        avl_update(node);
        uint32_t l = avl_depth(node->left);
        uint32_t r = avl_depth(node->right);
        AVLNode **from = nullptr;

        if (node->parent) {
            from = (node->parent->left == node) ? &node->parent->left
                                                : &node->parent->right;
        }

        if (l == r + 2) {
            node = avl_fix_left(node);
        } else if (l + 2 == r) {
            node = avl_fix_right(node);
        }

        if (!from) {
            return node;
        }

        *from = node;
        node = node->parent;
    }
}

/**
 * Delete a node
 * return **new** root of the tree
 */
AVLNode *avl_delete(AVLNode *node) {
    if (!(node->right)) {
        // no right subtree, replace the node with the left subtree
        AVLNode *parent = node->parent;
        if (node->left) {
            node->left->parent = parent;
        }
        if (parent) {
            (parent->left == node ? parent->left : parent->right) = node->left;
            return avl_rebalance(parent);
        } else {
            return node->left;
        }
    } else {
        // swap the node with its next sibling
        // the next smallest one that is larger than node
        AVLNode *victim = node->right;
        while (victim->left) {
            victim = victim->left;
        }
        AVLNode *root = avl_delete(victim);

        // swap?
        // since no data stored internally in AVLNode,
        // this is good enough
        *victim = *node;
        if (victim->left) {
            victim->left->parent = victim;
        }
        if (victim->right) {
            victim->right->parent = victim;
        }
        AVLNode *parent = node->parent;
        if (parent) {
            (parent->left == node ? parent->left : parent->right) = victim;
            return root;
        } else {
            // removing root?
            return victim;
        }
    }
}

AVLNode *avl_offset(AVLNode *node, int64_t offset) {
    // offset is number of nodes we walk to get to the destination node
    // offset can be negative if we need to walk upwards then go left
    int64_t pos = 0; // relative to the starting node
    while (offset != pos) {
        if (pos < offset && pos + avl_count(node->right) >= offset) {
            // target is inside the right subtree
            node = node->right;
            pos += avl_count(node->left) + 1;
        } else if (pos > offset && pos - avl_count(node->left) <= offset) {
            // target inside left subtree
            node = node->left;
            pos -= avl_count(node->right) + 1;
        } else {
            // go to parent
            AVLNode *parent = node->parent;
            if (!parent) {
                return nullptr;
            }

            if (parent->right == node) {
                pos -= avl_count(node->left) + 1;
            } else {
                pos += avl_count(node->right) + 1;
            }

            node = parent;
        }
    }

    return node;
}
