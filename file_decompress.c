#include "RandW.h"

int file_decompress(HANDLE hfile_read, HANDLE hfile_write, huffman_node* huffman_tree_root, unsigned long long original_size) {
    if (huffman_tree_root == NULL) return FILE_BROKEN_ERROR;
    if (huffman_tree_root->nodetype != TREE_NODE && huffman_tree_root->nodetype != LEAF_NODE) return FILE_BROKEN_ERROR;

    UINT8* in_buffer = (UINT8*)malloc(BLOCK_SIZE * 2);
    UINT8* out_buffer = (UINT8*)malloc(BLOCK_SIZE * 2);
    if (in_buffer == NULL || out_buffer == NULL) {
        free(in_buffer);
        free(out_buffer);
        return MEMORY_ALLOCATION_ERROR;
    }

    unsigned long long produced = 0;

    int in_buffer_size = 0;
    int in_buffer_index = 0;
    int out_buffer_size = 0;
    
    int bit_index = 8;
    UINT8 current_byte = 0;

    huffman_node* current_node = huffman_tree_root;

    for (;;) {
        if (current_node->nodetype == LEAF_NODE) {
            out_buffer[out_buffer_size++] = current_node->ch;
            produced++;
            if (produced >= original_size) {
                break;
            }
            if (out_buffer_size >= BLOCK_SIZE) {
                DWORD bytes_written = 0;
                if (!WriteFile(hfile_write, out_buffer, out_buffer_size, &bytes_written, NULL)) {
                    free(in_buffer);
                    free(out_buffer);
                    return FILE_WRITE_ERROR;
                }
                out_buffer_size = 0;
            }
            current_node = huffman_tree_root;
            continue;
        }

        if (bit_index >= 8) {
             if (in_buffer_index >= in_buffer_size) {
                DWORD bytes_read = 0;
                if (!ReadFile(hfile_read, in_buffer, BLOCK_SIZE, &bytes_read, NULL)) {
                    return FILE_ACCESS_ERROR;
                }
                if (bytes_read == 0) break;
                in_buffer_size = bytes_read;
                in_buffer_index = 0;
            }
            current_byte = in_buffer[in_buffer_index++];
            bit_index = 0;
        }

        int bit = (current_byte >> (7 - bit_index)) & 1;
        bit_index++;
        
        if (bit == 0) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }
    }

    DWORD bytes_written = 0;
    if (!WriteFile(hfile_write, out_buffer, out_buffer_size, &bytes_written, NULL)) {
        free(in_buffer);
        free(out_buffer);
        return FILE_WRITE_ERROR;
    }

    free(in_buffer);
    free(out_buffer);

    if (produced != original_size) return FILE_BROKEN_ERROR;

    return 0;
}