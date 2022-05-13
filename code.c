#include "code.h"

#include "defines.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

Code code_init(void) {
    Code c;
    c.top = 0;
    // Zero out each byte in bits
    for (int i = 0; i < MAX_CODE_SIZE; i++) {
        c.bits[i] = 0x0;
    }
    return c;
}

uint32_t code_size(Code *c) {
    return c->top;
}

bool code_empty(Code *c) {
    return c->top == 0;
}

bool code_full(Code *c) {
    // top references a bit, index references a byte
    return c->top == MAX_CODE_SIZE * 8;
}

bool code_push_bit(Code *c, uint8_t bit) {
    if (code_full(c)) {
        return false;
    }
    uint32_t idx = c->top / 8;
    uint32_t pos = c->top % 8;
    // for logical | to set, bit needs to be 0
    c->bits[idx] |= bit << pos;
    c->top++;
    return true;
}

bool code_pop_bit(Code *c, uint8_t *bit) {
    if (code_empty(c)) {
        return false;
    }
    c->top--;
    uint32_t idx = c->top / 8;
    uint32_t pos = c->top % 8;
    *bit = (c->bits[idx] >> pos) & 0x1;
    // clear bit in code for accurate print and pushing
    uint8_t mask = ~(0x1 << pos);
    c->bits[idx] &= mask;
    return true;
}

void code_print(Code *c) {
    printf("Code: ");
    for (uint32_t i = 0; i < code_size(c) / 8 + 1; i++) {
        printf("%x ", c->bits[i]);
    }
    printf("\n");
}
