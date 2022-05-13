#include "huffman.h"

#include "code.h"
#include "defines.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

#include <stdint.h>

Node *build_tree(uint64_t hist[static ALPHABET]) {
    PriorityQueue *pq = pq_create(ALPHABET);
    // enqueue all non-zero nodes
    for (int i = 0; i < ALPHABET; i++) {
        if (hist[i] > 0) {
            Node *n = node_create(i, hist[i]);
            enqueue(pq, n);
        }
    }

    // build the tree, second child (right) of root should be highest frequency
    // highest frequency appears on the lowest priority
    while (pq_size(pq) > 1) {
        Node *left;
        Node *right;
        dequeue(pq, &left);
        dequeue(pq, &right);
        Node *parent = node_join(left, right);
        enqueue(pq, parent);
    }

    if (pq_empty(pq)) {
        return (Node *) 0;
    }

    // Last node in queue is the root
    Node *root;
    dequeue(pq, &root);
    pq_delete(&pq);
    return root;
}
static Code c;
void build_codes(Node *root, Code table[static ALPHABET]) {
    uint8_t pop;
    if (root->left) {
        code_push_bit(&c, 0x0);
        build_codes(root->left, table);
    } else {
        // if it doesnt have a left child, it must be a leaf
        table[root->symbol] = c;
        code_pop_bit(&c, &pop);
        return;
    }

    // recurse right if there is a right child
    if (root->right) {
        code_push_bit(&c, 0x1);
        build_codes(root->right, table);
    }
    code_pop_bit(&c, &pop);
}

Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {
    Stack *s = stack_create(nbytes);
    for (uint16_t i = 0; i < nbytes; i++) {
        if (tree[i] == 'L') {
            // Leaf node, make a node and push with following symbol
            i++;
            Node *n = node_create(tree[i], 0);
            stack_push(s, n);
        } else if (tree[i] == 'I') {
            // Build interior node
            Node *left;
            Node *right;
            // When popping, the first child is right
            stack_pop(s, &right);
            stack_pop(s, &left);
            Node *parent;
            parent = node_join(left, right);
            stack_push(s, parent);
        }
    }
    // If tree dump is constructed properly,
    // there should be a single root node on the stack
    Node *root;
    stack_pop(s, &root);
    stack_delete(&s);
    return root;
}

void delete_tree(Node **root) {
    if ((*root)->left) {
        delete_tree(&(*root)->left);
    }
    if ((*root)->right) {
        delete_tree(&(*root)->right);
    }
    node_delete(root);
    root = NULL;
}
