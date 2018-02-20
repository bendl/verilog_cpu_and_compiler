//
// Created by BDL on 19/02/2018.
//


#include "adt/list.h"
#include <stdlib.h>
#include <assert.h>

void 
append_ll_item(struct list_item *head, void *val)
{
        struct list_item *current = head;
        struct list_item *item;

        // TODO: Maybe alloc list if head is null instead of error
        assert(current != NULL && "Linked list head is NULL!");

        if (current->value == NULL) {
                current->value = val;
                return;
        }

        while (current->next != NULL) {
                current = current->next;
        }

        item            = calloc(1, sizeof(*item));
        item->value     = val;
        current->next   = item;
}

struct list_item *
append_ll_item_head(struct list_item *head, void *val)
{
        if (head == NULL) {
                head        = calloc(1, sizeof(*head));
                head->value = val;
                return head;
        }

        // If the head is empty, return it
        if (head->value == NULL) {
                head->value = val;
                return head;
        }

        struct list_item *new_head = (struct list_item *)calloc(1, sizeof(*new_head));
        new_head->next  = head;
        new_head->value = val;
        return new_head;
}