#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "constants.h"
#include <cstdint>

// pointer arithmetics to convert the pointer to HNode to pointer to Entry
// using `typeof` gives me the following error:
// `ISO C++ forbids declaration of ‘typeof’ with no type [-fpermissive]`
#define container_of(ptr, type, member)                                        \
    ({                                                                         \
        const decltype(((type *)0)->member) *__mptr = (ptr);                   \
        (type *)((char *)__mptr - offsetof(type, member));                     \
    })
/**
 * `offsetof` is a macro
 * expands to an integral constant expression of type size_t
 * offset of from the beginning of an object of specified type to its specified
 * subobject
 * **noexcept**
 */

struct HNode {
    HNode *next = NULL;
    uint64_t hcode = 0;
};

struct HTable {
    HNode **table = NULL;
    size_t mask = 0;
    size_t size = 0;
};

struct HMap {
    HTable ht_to;
    HTable ht_from;
    size_t resizing_pos = 0;
};

void h_init(HTable *htable, size_t n);

/**
 * Insertion
 */
void h_insert(HTable *htable, HNode *node);

HNode **h_lookup(HTable *htable, HNode *key, bool (*cmp)(HNode *, HNode *));

HNode *h_detach(HTable *htable, HNode **from);

// scan through the entire hashtable and call f on every node
void h_scan(HTable *table, void (*f)(HNode *, void *), void *arg);

size_t hm_size(HMap *hmap);

void hm_help_resizing(HMap *hmap);

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*cmp)(HNode *, HNode *));

void hm_start_resizing(HMap *hmap);

void hm_insert(HMap *hmap, HNode *node);

HNode *hm_pop(HMap *hmap, HNode *key, bool (*cmp)(HNode *, HNode *));

void hm_destroy(HMap *hmap);

#endif /* HASHTABLE_H */
