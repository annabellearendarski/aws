/*
 * Copyright (c) 2009-2017 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 *
 *
 * Doubly-linked list.
 *
 * 
 */

#ifndef LIST_H
#define LIST_H
#include <stdbool.h>
#define structof(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))


struct list {
   struct list *prev;
   struct list *next;
};


/*
 * Initialize a list.
 */
static inline void
list_init(struct list *list)
{
    list->prev = list;
    list->next = list;
}

/*
 * Add a node between two nodes.
 *
 * This function is private.
 */
static inline void
list_add(struct list *prev, struct list *next, struct list *node)
{
    next->prev = node;
    node->next = next;

    prev->next = node;
    node->prev = prev;
}

/*
 * Insert a node at the head of a list.
 * list->next is pointing to the head of the list
 */
static inline void
list_insert_head(struct list *list, struct list *node)
{
    list_add(list, list->next, node);
}

/*
 * Return the node next to the given node.
 */
static inline struct list *
list_next(const struct list *node)
{
    return node->next;
}

/*
 * Return the first node of a list.
 */
static inline struct list *
list_first(const struct list *list)
{
    return list->next;
}

/*
 * Return true if node is invalid and denotes one of the ends of the list.
 */
static inline bool
list_end(const struct list *list, const struct list *node)
{
    return list == node;
}

/*
 * Remove a node from a list.
 *
 * After completion, the node is stale.
 */
static inline void
list_remove(struct list *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

#define list_entry(node, type, member) structof(node, type, member)

/*
 * Get the first entry of a list.
 */
#define list_first_entry(list, type, member) \
    list_entry(list_first(list), type, member)

/*
 * Get the entry next to the given entry.
 */
#define list_next_entry(entry, member) \
    list_entry(list_next(&(entry)->member), typeof(*(entry)), member)

/*
 * Forge a loop to process all entries of a list.
 *
 * The entry node must not be altered during the loop.
 */
#define list_for_each_entry(list, entry, member)                    \
for (entry = list_first_entry(list, typeof(*entry), member);        \
     !list_end(list, &entry->member);                               \
     entry = list_next_entry(entry, member))
#endif /* LIST_H */
