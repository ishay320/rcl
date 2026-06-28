// FIXME: right now, the use of the main node is only for typechecking and its
// confusing
#ifndef QUEUE_LL_H
#define QUEUE_LL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../link_list/link_list.h"

/*
 * Generic linked-list queue via macros and header.
 * Define a node struct with a `next` pointer and a `data` field, then use
 * qe_init_header to allocate the queue and get back a typed pointer:
 *
 *   typedef struct my_node { struct my_node* next; int data; } my_node;
 *
 *   my_node* q = qe_init_header(my_node);
 *   ...
 *   qe_destroy_header(q);
 */

/**
 * @brief Internal header stored before the first node in the allocation
 *
 * Allocated via qe_init_header; the flexible array member `data` is cast
 * to the user's node type so the header and first node share one allocation.
 */
typedef struct {
    void* front;
    void* back;
    int size;
    char data[];
} queue;

/**
 * @brief Allocate a queue header and return a pointer to the embedded
 * first-node slot
 *
 * @param T The user's node type
 * @return Pointer to the embedded T slot inside the header, or NULL on
 * allocation failure
 */
#define qe_init_header(T)                                      \
    ({                                                         \
        T* _ret        = NULL;                                 \
        queue* _header = calloc(1, sizeof(T) + sizeof(queue)); \
        if (_header == NULL) {                                 \
            fprintf(stderr, "Memory allocation failed!\n");    \
        } else {                                               \
            _ret = (T*)(_header->data);                        \
        }                                                      \
        _ret;                                                  \
    })

/**
 * @brief Push a new node to the back of the queue
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @param d The data value to be pushed into the queue (assigned to the new
 * node's data field)
 * @return true if the node was allocated and pushed successfully, false on
 * allocation failure
 */
#define qe_push(q, d)                                       \
    ({                                                      \
        bool __ret         = false;                         \
        typeof(q) new_node = calloc(1, sizeof(*new_node));  \
        if (new_node == NULL) {                             \
            fprintf(stderr, "Memory allocation failed!\n"); \
        } else {                                            \
            __ret          = true;                          \
            queue* _header = qe_get_header(q);              \
            new_node->data = d;                             \
            if (_header->back == NULL) {                    \
                _header->back  = new_node;                  \
                _header->front = new_node;                  \
            } else {                                        \
                typeof(q) back = _header->back;             \
                ll_insert_next(back, new_node);             \
                _header->back = new_node;                   \
            }                                               \
            _header->size++;                                \
        }                                                   \
        __ret;                                              \
    })

/**
 * @brief Pop the front node from the queue
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @param out Variable to store the popped front node's data
 * @return true if an node was popped successfully, false if the queue is empty
 */
#define qe_pop(q, out)                                        \
    ({                                                        \
        bool __ret     = false;                               \
        queue* _header = qe_get_header(q);                    \
        if (_header->front != NULL) {                         \
            __ret          = true;                            \
            typeof(q) __f  = _header->front;                  \
            (out)          = __f->data;                       \
            _header->front = __f->next;                       \
            if (_header->front == NULL) _header->back = NULL; \
            free(__f);                                        \
            _header->size--;                                  \
        }                                                     \
        __ret;                                                \
    })

/**
 * @brief Peek at the front node without removing it
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @param out Variable to store the front node's data (only written if queue is
 * not empty)
 * @return Pointer to the front node, or NULL if the queue is empty
 */
#define qe_front(q, out)                                 \
    ({                                                   \
        queue* _header = qe_get_header(q);               \
        if (_header->front) {                            \
            (out) = ((typeof(q))(_header->front))->data; \
        }                                                \
        (typeof(q))(_header->front);                     \
    })

/**
 * @brief Get a pointer to the front node without modifying the queue
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @return Pointer to the front node, or NULL if the queue is empty
 */
#define qe_front_node(q)                   \
    ({                                     \
        queue* _header = qe_get_header(q); \
        _header->front;                    \
    })

/**
 * @brief Peek at the back node without removing it
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @param out Variable to store the back node's data (only written if queue is
 * not empty)
 * @return Pointer to the back node, or NULL if the queue is empty
 */
#define qe_back(q, out)                                 \
    ({                                                  \
        queue* _header = qe_get_header(q);              \
        if (_header->back) {                            \
            (out) = ((typeof(q))(_header->back))->data; \
        }                                               \
        ((typeof(q))_header->back);                     \
    })

/**
 * @brief Get a pointer to the back node without modifying the queue
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @return Pointer to the back node, or NULL if the queue is empty
 */
#define qe_back_node(q)                    \
    ({                                     \
        queue* _header = qe_get_header(q); \
        _header->back;                     \
    })

/**
 * @brief Check if the queue is empty
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @return true if the queue contains no nodes, false otherwise
 */
#define qe_is_empty(q) (qe_get_header(q)->front == NULL)

/**
 * @brief Get the number of nodes in the queue
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @return The current number of nodes in the queue
 */
#define qe_size(q) (qe_get_header(q)->size)

/**
 * @brief Remove and free all nodes, resetting the queue to empty
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 */
#define qe_clear(q)                           \
    ({                                        \
        queue* _header = qe_get_header(q);    \
        while (_header->front != NULL) {      \
            typeof(q) front = _header->front; \
            _header->front  = front->next;    \
            free(front);                      \
        }                                     \
        _header->back = NULL;                 \
        _header->size = 0;                    \
    })

/**
 * @brief Recover the queue header pointer from any node pointer in the queue
 *
 * @note Relies on the node being the embedded slot inside the header allocation
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @return Pointer to the queue header
 */
#define qe_get_header(q)                                                \
    ({                                                                  \
        queue* __header = (queue*)((char*)(q) - offsetof(queue, data)); \
        __header;                                                       \
    })

/**
 * @brief Free the entire queue allocation, including all nodes and the header
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 */
#define qe_destroy_header(q)        \
    ({                              \
        if (q) {                    \
            qe_clear(q);            \
            free(qe_get_header(q)); \
        }                           \
        q = NULL;                   \
    })

/**
 * @brief Iterate over all nodes in the queue without modifying it
 *
 * @param q Pointer to the base queue node (returned by qe_init_header)
 * @param i_name Name of the iterator variable
 */
#define qe_for_each(q, i_name) for (typeof(*q)* i_name = (typeof(q))qe_front_node(q); i_name != NULL; i_name = i_name->next)

#endif  // QUEUE_LL_H
