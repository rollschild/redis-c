#include "zset.h"
#include "avl.h"
#include "hashtable.h"
#include "utils.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static uint32_t min(size_t lhs, size_t rhs) { return lhs < rhs ? lhs : rhs; }

ZNode *znode_new(const char *name, size_t len, double score) {
    ZNode *node = (ZNode *)malloc(sizeof(ZNode) + len);
    assert(node);

    avl_init(&node->tree);
    node->hmap.next = nullptr;
    node->hmap.hcode = str_hash((uint8_t *)name, len);
    node->score = score;
    node->len = len;
    memcpy(&node->name[0], name, len);
    return node;
}

void tree_add(ZSet *zset, ZNode *node) {
    if (!(zset->tree)) {
        zset->tree = &(node->tree);
        return;
    }

    AVLNode *curr = zset->tree;
    while (true) {
        AVLNode **from = zless(&node->tree, curr) ? &curr->left : &curr->right;

        if (!*from) {
            *from = &(node->tree);
            node->tree.parent = curr;
            zset->tree = avl_rebalance(&(node->tree));
            break;
        }

        curr = *from;
    }
}

bool zless(AVLNode *lhs, double score, const char *name, size_t len) {
    ZNode *zl = container_of(lhs, ZNode, tree);
    if (zl->score != score) {
        return zl->score < score;
    }

    int rv = memcmp(zl->name, name, min(zl->len, len));

    if (rv != 0) {
        return rv < 0;
    }

    return zl->len < len;
}

bool zless(AVLNode *lhs, AVLNode *rhs) {
    ZNode *zr = container_of(rhs, ZNode, tree);
    return zless(lhs, zr->score, zr->name, zr->len);
}

void zset_update(ZSet *zset, ZNode *node, double score) {
    if (node->score == score) {
        return;
    }

    zset->tree = avl_delete(&(node->tree));
    node->score = score;
    avl_init(&(node->tree));
    tree_add(zset, node);
}

bool zset_add(ZSet *zset, const char *name, size_t len, double score) {
    ZNode *node = zset_lookup(zset, name, len);
    if (node) {
        zset_update(zset, node, score);
        return false;
    } else {
        node = znode_new(name, len, score);
        hm_insert(&(zset->hmap), &(node->hmap));
        tree_add(zset, node);
        return true;
    }
}

bool hcmp(HNode *node, HNode *key) {
    if (node->hcode != key->hcode) {
        return false;
    }

    ZNode *znode = container_of(node, ZNode, hmap);
    HKey *hkey = container_of(key, HKey, node);

    if (znode->len != hkey->len) {
        return false;
    }

    return 0 == memcmp(znode->name, hkey->name, znode->len);
}

ZNode *zset_lookup(ZSet *zset, const char *name, size_t len) {
    if (!(zset->tree)) {
        return nullptr;
    }

    HKey key;
    key.node.hcode = str_hash((uint8_t *)name, len);
    key.name = name;
    key.len = len;

    HNode *found = hm_lookup(&(zset->hmap), &(key.node), &hcmp);

    if (!found) {
        return nullptr;
    }

    return container_of(found, ZNode, hmap);
}

ZNode *zset_query(ZSet *zset, double score, const char *name, size_t len,
                  int64_t offset) {
    AVLNode *found = nullptr;
    AVLNode *curr = zset->tree;

    while (curr) {
        if (zless(curr, score, name, len)) {
            curr = curr->right;
        } else {
            found = curr;
            curr = curr->left;
        }
    }

    if (found) {
        found = avl_offset(found, offset);
    }

    return found ? container_of(found, ZNode, tree) : nullptr;
}

ZNode *zset_pop(ZSet *zset, const char *name, size_t len) {
    if (!(zset->tree)) {
        return nullptr;
    }

    HKey key;
    key.node.hcode = str_hash((uint8_t *)name, len);
    key.name = name;
    key.len = len;
    HNode *found = hm_pop(&zset->hmap, &key.node, &hcmp);

    if (!found) {
        return nullptr;
    }

    ZNode *node = container_of(found, ZNode, hmap);
    zset->tree = avl_delete(&node->tree);
    return node;
}

void znode_del(ZNode *node) { free(node); }

void tree_dispose(AVLNode *node) {
    if (!node)
        return;

    tree_dispose(node->left);
    tree_dispose(node->right);

    znode_del(container_of(node, ZNode, tree));
}

void zset_dispose(ZSet *zset) {
    tree_dispose(zset->tree);
    hm_destroy(&zset->hmap);
}
