#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *n = (Node *) malloc(sizeof(Node));
    if (n) {
        n->symbol = symbol;
        n->frequency = frequency;
        n->left = NULL;
        n->right = NULL;
    }
    return n;
}

void node_delete(Node **n) {
    if (*n) {
        free(*n);
        *n = NULL;
    }
}

// Function to aid with swapping node values
void node_swap(Node *a, Node *b) {
    Node t = *b;
    *b = *a;
    *a = t;
}

Node *node_join(Node *left, Node *right) {
    uint64_t freq = left->frequency + right->frequency;
    Node *parent = node_create('$', freq);
    parent->left = left;
    parent->right = right;
    return parent;
}

void node_print(Node *n) {
    printf("Node '%c' freq: %" PRIu64 "\n", n->symbol, n->frequency);
}
