#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <commdlg.h>
#define BLOCK_SIZE 4096
#define COMPRESS 1
#define DECOMPRESS 0
#define FILE_NOT_FOUND -1
#define FILE_ACCESS_ERROR -2
#define FILE_WRITE_ERROR -3
#define TREE_NODE 0
#define LEAF_NODE 1
#define MAX_HUFFMAN_CODE_SIZE 256
#define BYTESIZE 8
#define MEMORY_ALLOCATION_ERROR -4
#define FILE_BROKEN_ERROR -5
typedef struct{
    UINT8 code[MAX_HUFFMAN_CODE_SIZE / 8];
    int length;
    unsigned long long freq;
} huffman_code;

typedef struct huffman_node{
    unsigned long long weight;
    char nodetype;
    struct huffman_node* left;
    struct huffman_node* right;
    unsigned char ch;
} huffman_node;

struct huffman_file_header{
    unsigned long long original_file_size;
    huffman_code huffman_table[256];
};