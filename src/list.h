struct DList {
    DList *prev = nullptr;
    DList *next = nullptr;
};

inline void dlist_init(DList *node) { node->prev = node->next = node; }

inline bool dlist_is_empty(DList *node) { return node == node->next; }

inline void dlist_detach(DList *node) {
    DList *prev = node->prev;
    DList *next = node->next;
    prev->next = next;
    next->prev = prev;
}

inline void dlist_insert_before(DList *target, DList *to_be_inserted) {
    DList *prev = target->prev;
    prev->next = to_be_inserted;
    to_be_inserted->prev = prev;
    to_be_inserted->next = target;
    target->prev = to_be_inserted;
}
