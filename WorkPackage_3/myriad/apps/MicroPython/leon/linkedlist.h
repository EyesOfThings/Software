#include <stdio.h>

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#define LL_VERSION "1.0.0-b5"

struct node {
    void *value;
    struct node *prev;
    struct node *next;
};
typedef struct node item;

typedef struct {
    item *head;
    item *leaf;
    item *current;
    unsigned int size;
} linkedlist;

linkedlist *ll_create();
int ll_push_first(linkedlist *, void *); /* O(1) */
int ll_push_last(linkedlist *, void *); /* O(1) */
void *ll_pop_first(linkedlist *); /* O(1) */
void *ll_pop_last(linkedlist *); /* O(1) */
int ll_pop_all(linkedlist *); /* O(n) */
void *ll_get_index(linkedlist *list, unsigned int index);
void *ll_get_first(linkedlist *); /* O(1) */
void *ll_get_last(linkedlist *); /* O(1) */
void *ll_get_next(linkedlist *); /* O(1) */
void *ll_get_prev(linkedlist *); /* O(1) */
int ll_exists(linkedlist *, void *, int (*)(void *, void *)); /* O(n) */
int ll_item_position(linkedlist *, void *, int (*)(void *, void *)); 
int ll_remove_item(linkedlist *, void *, int (*)(void *, void *));
int ll_sort(linkedlist *, int (*)(void *, void *)); /* O(n^2) */
int ll_clear(linkedlist *, void (*)(void *));
int ll_delete(linkedlist *, void (*)(void *));
void ll_print(linkedlist *, FILE *, void (*)(FILE *, void *));
void ll_print_filter(linkedlist *, FILE *, void (*)(FILE *, void *), int (*)(void *));
void ll_dump(linkedlist *, FILE *, void (*)(FILE *, void *));

#endif