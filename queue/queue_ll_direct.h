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
 *   qd_push(&q, 42);
 *   int value;
 *   qd_pop(&q, &value);
 */

/**
 * @brief Push a new node to the back of the queue
 *
 * @param q Pointer to the queue
 * @param d The data value to be pushed into the queue
 * @return true if the node was allocated and pushed successfully, false on allocation failure
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

/**
 * @brief Pop the front node from the queue
 *
 * @param q Pointer to the queue
 * @param out Variable to store the popped front node's data
 * @return true if an node was popped successfully, false if the queue is empty
 */
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

/**
 * @brief Get the data of the front node without removing it
 *
 * @param q Pointer to the queue
 * @param out Variable to store the front node's data
 * @return Pointer to the front node, or NULL if the queue is empty
 */
#define qd_front(q, out)                          \
    ({                                            \
        if ((q)->front) (out) = (q)->front->data; \
        (q)->front;                               \
    })

/**
 * @brief Get the data of the back node without removing it
 *
 * @param q Pointer to the queue
 * @param out Variable to store the back node's data
 * @return Pointer to the back node, or NULL if the queue is empty
 */
#define qd_back(q, out)                         \
    ({                                          \
        if ((q)->back) (out) = (q)->back->data; \
        (q)->back;                              \
    })

/**
 * @brief Check if the queue is empty
 *
 * @param q Pointer to the queue
 * @return true if the queue is empty, false otherwise
 */
#define qd_is_empty(q) ((q)->front == NULL)

/**
 * @brief Get the size of the queue
 *
 * @param q Pointer to the queue
 * @return The number of elements in the queue
 */
#define qd_size(q) ((q)->size)

/**
 * @brief Clear all elements from the queue
 *
 * @param q Pointer to the queue
 */
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
