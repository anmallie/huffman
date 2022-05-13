#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct PriorityQueue PriorityQueue;

struct PriorityQueue {
    uint32_t size;
    uint32_t capacity;
    Node **items;
};

PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    if (pq) {
        pq->size = 0;
        pq->capacity = capacity;
        pq->items = (Node **) calloc(capacity, sizeof(Node *));
        if (!pq->items) {
            free(pq);
            pq = NULL;
        }
    }
    return pq;
}

void pq_delete(PriorityQueue **q) {
    if ((*q) && (*q)->items) {
        free((*q)->items);
        free(*q);
        *q = NULL;
    }
}

bool pq_empty(PriorityQueue *q) {
    return q->size == 0;
}

bool pq_full(PriorityQueue *q) {
    return q->size == q->capacity;
}

uint32_t pq_size(PriorityQueue *q) {
    return q->size;
}

bool enqueue(PriorityQueue *q, Node *n) {
    if (pq_full(q)) {
        return false;
    }
    if (pq_empty(q)) {
        q->items[0] = n;
        q->size++;
        return true;
    }
    // should always take one complete pass of the list
    Node *curr = n;
    for (uint32_t i = 0; i < q->size; i++) {
        if (curr->frequency > (q->items[i])->frequency) {
            node_swap(curr, q->items[i]);
        }
    }
    q->items[q->size] = curr;
    q->size++;
    return true;
}

bool dequeue(PriorityQueue *q, Node **n) {
    if (pq_empty(q)) {
        return false;
    }
    q->size--;
    *n = q->items[q->size];
    return true;
}

void pq_print(PriorityQueue *q) {
    printf("Priority Queue of size: %" PRIu32 "\n", q->size);
    printf("----\n");
    for (uint32_t i = q->size; i > 0; i--) {
        node_print(q->items[i - 1]);
    }
}
