# Huffman Compression Algorithm encoder and decoder

These programs compress and decompress data using a Huffman algorithm, a static and lossless compression.  

### Note
The code in this repository was created for Prof. Long's CSE-13s class at UCSC.

The following files were given for the assignment and are not my work:
- entropy.c
- Header files {\*.h}

The rest of the code is my work unless stated otherwise.


## Files

- io.{c, h}       Implementation of the i/o, wrapper for <unistd.h> calls.

- node.{c, h}     Implementation of node ADT.

- stack.{c, h}    Implementation of the priority queue ADT.

- code.{c, h}     Implementation of the code ADT.

- stack.{c, h}    Implementation of the node stack ADT.

- huffman.{c, h}  Implementation of Huffman decoding and encoding logic.

- encode.c        Encoder program.

- decode.c        Decoder program.

- entropy.c       Program given by Prof. Long that calculates the entropy of data.

- header.h        File header struct definition.

- defines.h       Defining various macros used in the program.

- Makefile

## Makefile

### Build

        $ make {all, encode, decode, entropy}

## Cleaning

	$ make clean

## Running

        $ ./encode -[h] -[v] -[i input] -[o output]
        $ ./decode -[h] -[v] -[i input] -[o output]

### Options

* -h          Prints help message.

* -v          Enable printing compression statistics to stderr.

* -i infile   Specify input file to compress.

* -o outfile  Specify file to output compressed file.

