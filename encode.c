#include "code.c"
#include "defines.h"
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

void prep_hist(uint64_t *hist);
void fill_hist(int infile, uint64_t *hist);

Header make_header(int infile, uint64_t *hist);

void tree_dump(char *buf, Node *root);

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
            if (check_open(infile, optarg)) { 
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

    // Construct histogram
    uint64_t hist[ALPHABET];
    prep_hist(hist); //zeroes and adds min values
    fill_hist(infile, hist);

    // Construct Huffman tree and build code table
    Node *root = build_tree(hist);
    Code table[ALPHABET];
    build_codes(root, table);

    // Construct header and set perms in outfile, then write to outfile
    Header h = make_header(infile, hist);
    assert(fchmod(outfile, h.permissions) != -1);
    write_bytes(outfile, (uint8_t *) &h, sizeof(h));

    // Write tree to outfile (post-order traversal)
    char dump[h.tree_size];
    tree_dump(dump, root);
    write_bytes(outfile, (uint8_t *) dump, h.tree_size);

    // Write code for each symbol in infile then flush_codes
    lseek(infile, 0, SEEK_SET); //reset position in infile (from hist fill)
    uint8_t buf[BLOCK];
    int num_read = 0;
    while ((num_read = read_bytes(infile, buf, BLOCK)) != 0) {
        for (int i = 0; i < num_read; i++) {
            write_code(outfile, &table[buf[i]]);
        }
    }
    flush_codes(outfile);

    // Print compression stats
    if (verbose) {
        uint64_t unc = h.file_size; // uncompressed
        uint64_t comp = bytes_written; // compressed (extern from io)
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes\n", unc);
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes\n", comp);
        double space_saving = 100.0 * (1.0 - (comp / (double) unc));
        fprintf(stderr, "Space saving: %.2f%%\n", space_saving);
    }

    delete_tree(&root);
    close(infile);
    close(outfile);
    return EXIT_SUCCESS;
}

// zeroes histogram and adds min values
void prep_hist(uint64_t *hist) {
    for (int i = 0; i < ALPHABET; i++) {
        hist[i] = 0;
    }
    hist[0] = 1;
    hist[255] = 1;
}

void fill_hist(int infile, uint64_t *hist) {
    uint8_t buffer[BLOCK];
    int num_read = 0;
    while ((num_read = read_bytes(infile, buffer, BLOCK)) != 0) {
        for (int i = 0; i < num_read; i++) {
            hist[buffer[i]]++;
        }
    }
}

// Fills header with information, hist to count tree size
Header make_header(int infile, uint64_t *hist) {
    Header h;
    h.magic = MAGIC;

    struct stat statbuf; 
    assert(fstat(infile, &statbuf) != -1);
    h.permissions = statbuf.st_mode;

    h.tree_size = 0;
    // Tree size = 3x(unique symbols)-1
    for (int i = 0; i < ALPHABET; i++) {
        if (hist[i] != 0) {
            h.tree_size += 3;
        }
    }
    h.tree_size--;
    h.file_size = statbuf.st_size;
    return h;
}

// Fill buffer with symbolic huffman tree from post-order traversal
void tree_dump(char *buf, Node *root) {
    static int idx;
    if (root->left) {
        tree_dump(buf, root->left);
    }
    if (root->right) {
        tree_dump(buf, root->right);
    }

    if (!root->left && !root->right) { // leaf
        buf[idx] = 'L';
        buf[idx + 1] = root->symbol;
        idx += 2;
    } else { // interior
        buf[idx] = 'I';
        idx++;
    }
}

void print_help(char *path) {
    printf("SYNOPSIS\n");
    printf("  An encoder for Huffman compression.\n\n");
    printf("USAGE\n");
    printf("  %s [-h] [-v] [-i infile] [-o outfile]\n\n", path);
    printf("OPTIONS\n");
    printf("  -%-14s Program usage and help.\n", "h");
    printf("  -%-14s Print compression statistics to stderr.\n", "v");
    printf("  -%-14s Specify input file to compress.\n", "i infile");
    printf("  -%-14s Specify output file for compressed file.\n", "o outfile");
}

// returns non-zero if fd is an error code
int check_open(int fd, char *filename) {
    if (errno == EACCES) {
        fprintf(stderr, "Error: File %s cannot be accessed.\n", filename);
        return 1; // Error
    } else if (errno == ENOENT) {
        fprintf(stderr, "Error: File %s does not exist.\n", filename);
        return 1;
    } else if (errno == EEXIST) {
        fprintf(stderr, "Error: file %s already exists.\n", filename);
        return 1;
    } else if (fd < 0) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }
    return 0; // OK
}
