#ifndef QUEUE_LL_DIRECT_H
#define QUEUE_LL_DIRECT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Direct linked-list queue. No hidden header allocation.
 * User declares a node struct with `next` and `data`, and a queue struct
 * with `front`, `back`, `size`. Supports zero-initialization (= {0}).
 *
 *   typedef struct my_node { struct my_node* next; int data; } my_node;
 *   typedef struct { my_node* front; my_node* back; int size; } my_queue;
 *   my_queue q = {0};
 */

#define qd_push(q, d)                                           \
    ({                                                          \
        bool __ret                = false;                      \
        typeof((q)->front) __node = calloc(1, sizeof(*__node)); \
        if (__node == NULL) {                                   \
            fprintf(stderr, "Memory allocation failed!\n");     \
        } else {                                                \
            __ret        = true;                                \
            __node->data = (d);                                 \
            __node->next = NULL;                                \
            if ((q)->back == NULL) {                            \
                (q)->front = __node;                            \
                (q)->back  = __node;                            \
            } else {                                            \
                (q)->back->next = __node;                       \
                (q)->back       = __node;                       \
            }                                                   \
            (q)->size++;                                        \
        }                                                       \
        __ret;                                                  \
    })

#define qd_pop(q, out)                                \
    ({                                                \
        bool __ret = false;                           \
        if ((q)->front != NULL) {                     \
            __ret                  = true;            \
            typeof((q)->front) __f = (q)->front;      \
            (out)                  = __f->data;       \
            (q)->front             = __f->next;       \
            if ((q)->front == NULL) (q)->back = NULL; \
            free(__f);                                \
            (q)->size--;                              \
        }                                             \
        __ret;                                        \
    })

#define qd_front(q, out)                          \
    ({                                            \
        if ((q)->front) (out) = (q)->front->data; \
        (q)->front;                               \
    })

#define qd_back(q, out)                         \
    ({                                          \
        if ((q)->back) (out) = (q)->back->data; \
        (q)->back;                              \
    })

#define qd_is_empty(q) ((q)->front == NULL)
#define qd_size(q) ((q)->size)

#define qd_clear(q)                              \
    ({                                           \
        while ((q)->front != NULL) {             \
            typeof((q)->front) __f = (q)->front; \
            (q)->front             = __f->next;  \
            free(__f);                           \
        }                                        \
        (q)->back = NULL;                        \
        (q)->size = 0;                           \
    })

#endif  // QUEUE_LL_DIRECT_H
