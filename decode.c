#include "code.c"
#include "header.h"
#include "huffman.c"
#include "io.c"
#include "node.c"
#include "pq.c"
#include "stack.c"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // only use for printf, else use io.c
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OPTIONS "hvi:o:"

void print_help(char *path);
int check_open(int fd, char *filename);

int main(int argc, char **argv) {
    bool verbose = false;
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    // Parse options
    int opt;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': print_help(argv[0]); return EXIT_SUCCESS;
        case 'v': verbose = true; break;
        case 'i':
            infile = open(optarg, O_RDONLY); 
            if (check_open(infile, optarg)) { // returns 1 for error
                close(outfile); 
                return EXIT_FAILURE;
            }
            break;
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC);
            if (check_open(outfile, optarg)) {
                close(infile);
                return EXIT_FAILURE;
            }
            break;
        default: print_help(argv[0]); return EXIT_SUCCESS;
        }
    }

    // Read in header and check magic number
    Header h;
    read_bytes(infile, (uint8_t *) &h, sizeof(Header));
    if (h.magic != MAGIC) {
        fprintf(stderr, "Error: invalid file header\n");
        return EXIT_FAILURE;
    }

    // Set outfile perms from header
    fchmod(outfile, h.permissions);

    // Rebuild tree
    uint8_t read_tree[h.tree_size];
    read_bytes(infile, read_tree, h.tree_size);
    Node *root = rebuild_tree(h.tree_size, read_tree);

    // Read infile with read_bit and write symbols to outfile
    uint8_t bit = 0;
    uint8_t buffer[BLOCK];
    int idx = 0;
    Node *curr = root;
    while (bytes_written < h.file_size && read_bit(infile, &bit)) {
        if (bit == 0x0 && curr->left) {
            curr = curr->left;
        } else if (bit == 0x1 && curr->right) {
            curr = curr->right;
        }
        if (!curr->left && !curr->right) { // Found leaf
            buffer[idx] = curr->symbol;
            idx++;
            if (idx == BLOCK) { // buffer is full
                write_bytes(outfile, buffer, BLOCK);
                idx = 0;
            }
            curr = root;
        }
    }

    // Flush any remaining bytes in buffer
    uint64_t remaining = h.file_size - bytes_written;
    if (remaining) {
        write_bytes(outfile, buffer, remaining); 
    }

    // Print compression stats
    if (verbose) {
        uint64_t comp = bytes_read; // compressed size
        uint64_t decomp = bytes_written; // uncompressed size
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", comp);
        fprintf(stderr, "Decompressed file size: %" PRIu64 " bytes\n", decomp);
        double space_saving = 100.0 * (1.0 - (comp / (double) decomp));
        fprintf(stderr, "Space saving: %.2f%%\n", space_saving);
    }

    delete_tree(&root);
    close(infile);
    close(outfile);
    return EXIT_SUCCESS;
}

void print_help(char *path) {
    printf("SYNOPSIS\n");
    printf("  A decoder for Huffman compression.\n\n");
    printf("USAGE\n");
    printf("  %s [-h] [-v] [-i infile] [-o outfile]\n\n", path);
    printf("OPTIONS\n");
    printf("  -%-14s Program usage and help.\n", "h");
    printf("  -%-14s Print decompression statistics to stderr.\n", "v");
    printf("  -%-14s Specify input file to decompress.\n", "i infile");
    printf("  -%-14s Specify output file for decompresed file.\n", "o outfile");
}

//  Return 1 on error 
int check_open(int fd, char *filename) {
    if (errno == EACCES) {
        fprintf(stderr, "Error: File %s cannot be accessed.\n", filename);
        return 1; // Error
    } else if (errno == ENOENT) {
        fprintf(stderr, "Error: File %s does not exist.\n", filename);
        return 1;
    } else if (fd < 0) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }
    return 0; // OK
}
