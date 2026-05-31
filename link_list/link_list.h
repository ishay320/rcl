/*
Node type prefixes:
  ll - singly linked list: next only
*/

/**
 * @brief Link base to node (insert node after base)
 *
 * @param base The node after which to insert (pointer)
 * @param node The node to insert (pointer)
 */
#define ll_insert_next(base, node) \
    ({                             \
        (base)->next = (node);     \
        (base);                    \
    })

/**
 * @brief Check if list is empty (next is NULL)
 *
 * @param root The head node (value, not pointer)
 * @return Non-zero if empty, zero otherwise
 */
#define ll_is_empty(root) ((root).next == NULL)

/**
 * @brief Remove the node after the given node
 *
 * @note Cannot delete a node without its predecessor; asserts next != NULL
 * @param node The predecessor node (pointer)
 * @return The node that was removed from the link list
 */
#define ll_remove_next(node)                         \
    ({                                               \
        assert((node)->next != NULL);                \
        typeof((node)->next) n = (node)->next;       \
        (node)->next           = (node)->next->next; \
        n;                                           \
    })

/**
 * @brief Iterate over list nodes
 *
 * @param base The head node (value, not pointer); iteration starts from base itself
 * @param i_name Name of the pointer variable used as iterator
 */
#define ll_for_each(base, i_name) for (typeof(base)* i_name = &base; i_name != NULL; i_name = i_name->next)

#define ll_for_each_free(base, free_func) \
    ll_for_each(base, __i) { free_func(__i) }

/**
 * @brief Count nodes in the list
 *
 * @param head The head node (value, not pointer)
 * @return Number of nodes (size_t)
 */
#define ll_count(head)                        \
    ({                                        \
        size_t __count = 0;                   \
        ll_for_each(head, __i) { __count++; } \
        __count;                              \
    })

/**
 * @brief Find first node matching a predicate
 *
 * @param head The head node (value, not pointer)
 * @param pred Function pointer: int pred(node_type*) - return non-zero for match
 * @return Pointer to first matching node, or NULL if not found
 */
#define ll_find(head, pred)           \
    ({                                \
        typeof(head)* __found = NULL; \
        ll_for_each(head, __i) {      \
            if (pred(__i)) {          \
                __found = __i;        \
                break;                \
            }                         \
        }                             \
        __found;                      \
    })

/**
 * @brief Insert node at the tail of the list
 *
 * @param head The head node (pointer)
 * @param node The node to append (pointer)
 */
#define ll_insert_tail(head, node)                  \
    ({                                              \
        typeof(head) __i = head;                    \
        for (; __i->next != NULL; __i = __i->next); \
        __i->next = node;                           \
    })
