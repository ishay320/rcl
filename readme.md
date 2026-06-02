# Reusable C Libraries

This repo contains my standard libraries in c that help me not write everything from scratch every time because c...

---

## Array (Dynamic)

Generic dynamic array via macros. Bring-your-own struct with `size`, `capacity`, `data` fields.

- Auto-resizes (doubles capacity, starts at 8)
- Type-agnostic, works with any typed struct
- `da_append` returns `bool` (false on alloc fail)
- Lightweight, Focused on speed and efficiency

```c
#include "arrays/dynamic_array.h"

typedef struct {
    size_t size;
    size_t capacity;
    int* data;
} da_i;

int main(void)
{
    da_i arr = {0};
    for (int i = 0; i < 50; i++) {
        da_append(arr, i);
    }
    for (size_t i = 0; i < arr.size; i++) {
        printf("%d\n", arr.data[i]);
    }
    da_destroy(&arr);
    return 0;
}
```

---

## Stack

Generic dynamic stack built on top of dynamic array.

```c
#include "stack/dynamic_stack.h"

typedef struct {
    size_t size;
    size_t capacity;
    int* data;
} stack_i;

int main(void)
{
    stack_i s = {0};
    ds_push(s, 10);
    ds_push(s, 20);
    int top = ds_pop(s); // 20
    da_destroy(&s);
    return 0;
}
```

---

## Queue (Array based)

Fixed capacity circular buffer queue. Bring-your-own struct.

```c
#include "queue/queue_a.h"

typedef struct {
    size_t size;
    size_t cap;
    size_t start;
    size_t end;
    int* data;
} int_queue;

int main(void)
{
    int_queue q = qa_init(int_queue, 8);
    qa_push(q, 1);
    qa_push(q, 2);
    int val;
    qa_pop(q, val); // val = 1
    qa_destroy(q);
    return 0;
}
```

---

## Linked List (Singly)

Intrusive singly-linked list - embed `next` pointer in your struct.

```c
#include "link_list/link_list.h"

typedef struct Node {
    int val;
    struct Node* next;
} Node;

int main(void)
{
    Node head = {0};
    Node n1 = {.val = 42};
    ll_insert_next(&head, &n1);
    ll_for_each(head, it) {
        printf("%d\n", it->val);
    }
    return 0;
}
```

---

## Graph

Adjacency list graph backed by dynamic arrays.

> NOTE: this is not ok, all the code will be refactor

**Features:**

- Dynamic node/edge management
- Type-agnostic node data

> `graph_free` does not free node data - clean it up yourself.

```c
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    struct graph* g = graph_init();

    int* d1 = malloc(sizeof(int)); *d1 = 42;
    int* d2 = malloc(sizeof(int)); *d2 = 99;

    size_t id1 = graph_node_push(g, d1);
    size_t id2 = graph_node_push(g, d2);
    graph_eadge_create(g, id1, id2);

    struct node* n = graph_node_get(g, id1);
    printf("Node %zu: %d\n", n->id, *((int*)n->data));

    graph_free(g);
    free(d1);
    free(d2);
    return 0;
}
```

---

## ANSI

Header-only terminal escape code macros. WIP - the movement need rework.

**Features:** cursor movement, screen/line erase, styling (bold, colors, etc.)

```c
#include "ansi.h"

printf(AN_BOLD "Bold text" AN_RESET "\n");
printf(AN_RED AN_UNDERLINE  "Red underline text" AN_RESET "\n");
printf(AN_MOVE(5, 10) "Line 5, col 10\n");
```
