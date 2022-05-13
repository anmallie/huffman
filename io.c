#include "code.h"
#include "defines.h"

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

uint64_t bytes_read = 0;
uint64_t bytes_written = 0;
static uint8_t code_buffer[BLOCK];
static uint32_t code_idx;

int read_bytes(int infile, uint8_t *buf, int nbytes) {
    int to_read = nbytes;
    while (to_read) {
        int num_read = read(infile, &buf[nbytes - to_read], to_read);
        to_read -= num_read;
        bytes_read += num_read;
        if (!num_read) {
            break; // EOF reached
        }
    }
    return nbytes - to_read;
}

int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int to_write = nbytes;
    while (to_write) {
        int num_written = write(outfile, &buf[nbytes - to_write], to_write);
        to_write -= num_written;
        bytes_written += num_written;
        if (!num_written) {
            break; // No bytes written
        }
    }
    return nbytes - to_write;
}

// Returns true if there are more bits to read
bool read_bit(int infile, uint8_t *bit) {
    static uint8_t buffer[BLOCK];
    static uint32_t bit_idx;
    static uint32_t cap; // max bit

    // initial fill for buffer
    if (!cap) {
        cap = read_bytes(infile, buffer, BLOCK) * 8;
        bit_idx = 0;
        if (!cap) {
            return false;
        } // no read bytes? EOF
    }

    uint32_t byte = bit_idx / 8;
    uint32_t pos = bit_idx % 8;
    *bit = (buffer[byte] >> pos) & 0x1;
    bit_idx++;

    // bottom check/fill to make sure that return statement is
    // preventative vs reactive
    if (bit_idx == cap) {
        cap = read_bytes(infile, buffer, BLOCK) * 8;
        bit_idx = 0;
    }
    return cap > 0;
}

// Uses static code_buffer and code_idx, idx referring to bit
void write_code(int outfile, Code *c) {
    for (uint32_t i = 0; i < code_size(c); i++) {
        uint8_t byte = c->bits[i / 8];
        uint8_t bit = (byte >> i % 8) & 0x1;
        uint8_t pos = code_idx % 8;
        code_buffer[code_idx / 8] |= bit << pos;
        code_idx++;

        if (code_idx == BLOCK * 8) {
            // Buffer is full, write and clear
            write_bytes(outfile, code_buffer, BLOCK);
            for (int i = 0; i < BLOCK; i++) {
                code_buffer[i] = 0x0;
            }
            code_idx = 0;
        }
    }
}

// Write the remaining bytes in buffer
void flush_codes(int outfile) {
    uint32_t bytes = (code_idx / 8) + 1;
    // Extra bits in last byte should already be cleared from write_code
    write_bytes(outfile, code_buffer, bytes);
}
