#ifndef ZSET_H
#define ZSET_H

#include "avl.h"
#include "hashtable.h"
#include <cstddef>
#include <cstdint>

struct ZSet {
    AVLNode *tree = nullptr;
    HMap hmap;
};

/**
 * a pair of (score, name) node
 * supports query/update by the sorting key, or by the `name`
 */
struct ZNode {
    AVLNode tree;
    HNode hmap;
    double score = 0;
    size_t len = 0;
    char name[0]; // ???
};

/**
 * helper structure for the hashtable lookup
 */
struct HKey {
    HNode node;
    const char *name = NULL;
    size_t len = 0;
};

ZNode *znode_new(const char *name, size_t len, double score);

void tree_add(ZSet *zset, ZNode *node);

/**
 * compare by the (score, name) tuple
 */
bool zless(AVLNode *lhs, double score, const char *name, size_t len);
bool zless(AVLNode *lhs, AVLNode *rhs);

/**
 * update the score of an existing node
 * (AVL Tree re-insertion)
 */
void zset_update(ZSet *zset, ZNode *node, double score);

/**
 * add a new (score, name) tuple, or
 * update the score of the existing tuple
 */
bool zset_add(ZSet *zset, const char *name, size_t len, double score);

bool hcmp(HNode *node, HNode *key);

/**
 * Lookup by name
 */
ZNode *zset_lookup(ZSet *zset, const char *name, size_t len);

/**
 * Primary usecase of the sorted sets: range query
 * Find the _smallest_ (score, name) tuple that is >= the argument,
 * then offset relative to it
 */
ZNode *zset_query(ZSet *zset, double score, const char *name, size_t len,
                  int64_t offset);

/**
 * Deletion by name
 */
ZNode *zset_pop(ZSet *zset, const char *name, size_t len);

void tree_dispose(AVLNode *node);
void zset_dispose(ZSet *zset);

void znode_del(ZNode *node);

#endif /* ZSET_H */
