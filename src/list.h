#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "macro.h"

struct list{
    struct list *prev;
    struct list *next;
};

static inline void
list_init(struct list *tail)
{
    tail->prev = tail;
    tail->next = tail;
}

static inline void
list_add(struct list *prev, struct list *tail, struct list *node)
{
    tail->prev = node;
    node->next = tail;

    prev->next = node;
    node->prev = prev;
}

/*
 * Insert a node at the head of a list.
 */
static inline void
list_insert_head(struct list *list, struct list *node)
{
    list_add(list, list->next, node);
}

static inline void
list_insert_tail(struct list *tail, struct list *node)
{
    list_add(tail->prev, tail, node);
}

static inline struct list *
list_first(const struct list *list)
{
    return list->next;
}

static inline struct list *
list_last(struct list *list)
{
    return list->prev;
}

static inline struct list *
list_prev(struct list *node)
{
    return node->prev;
}

static inline bool
list_end(const struct list *list, const struct list *node)
{
    return list == node;
}

static inline bool
list_empty(const struct list *list)
{
    return list == list->next;
}

static inline void
list_remove(struct list *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

#define list_entry(node, type, member) structof(node, type, member)

/*
 * Get the first entry of a list.
 */
#define list_first_entry(list, type, member) \
    list_entry(list_first(list), type, member)

/*
 * Get the last entry of a list.
 */
#define list_last_entry(list, type, member) \
    list_entry(list_last(list), type, member)

/*
 * Return the node next to the given node.
 */
static inline struct list *
list_next(const struct list *node)
{
    return node->next;
}

/*
 * Get the entry previous to the given entry.
 */
#define list_prev_entry(entry, member) \
    list_entry(list_prev(&(entry)->member), typeof(*(entry)), member)

#define list_next_entry(entry, member) \
    list_entry(list_next(&(entry)->member), typeof(*(entry)), member)

#define list_for_each_entry_reverse(list, entry, member)            \
for (entry = list_last_entry(list, typeof(*entry), member);         \
     !list_end(list, &entry->member);                               \
     entry = list_prev_entry(entry, member))

/*
 * Forge a loop to process all entries of a list.
 *
 * The entry node must not be altered during the loop.
 */
#define list_for_each_entry(list, entry, member)                    \
for (entry = list_first_entry(list, typeof(*entry), member);        \
     !list_end(list, &entry->member);                               \
     entry = list_next_entry(entry, member))

#endif /*LIST_H*/
