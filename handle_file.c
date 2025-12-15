#include "RandW.h"

int file_compress(HANDLE hfile_read, HANDLE hfile_write, huffman_code huffman_table[256]);
int file_decompress(HANDLE hfile_read, HANDLE hfile_write, huffman_node* huffman_tree_root, unsigned long long original_size);
int build_feq_table(HANDLE hfile, huffman_code table[256], unsigned long long* total_bytes);
huffman_node* build_huffman_tree(huffman_code table[256]);
void free_huffman_tree(huffman_node* root);
int build_huffman_table(huffman_node* root, huffman_code table[256]);

int handle_file(LPCWSTR file_name,INT8 mode){
    HANDLE hfile_read = CreateFileW(
        file_name,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if ( hfile_read == INVALID_HANDLE_VALUE ) return FILE_NOT_FOUND;

    LPCWSTR out_file_name;
    if (mode == COMPRESS){
        out_file_name = L"compressed_file.huff";
    } else {
        out_file_name = L"decompressed_file.dat";
    }
    HANDLE hfile_write = CreateFileW(
        out_file_name,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if ( hfile_write == INVALID_HANDLE_VALUE ) {
        CloseHandle(hfile_read);
        return FILE_ACCESS_ERROR;
    }

    DWORD bytes_read;
    DWORD bytes_written;

    // -------------------------- Compression Logic --------------------------
    if (mode == COMPRESS) {
        huffman_code* huffman_table = (huffman_code*)malloc(256 * sizeof(huffman_code));
        if (huffman_table == NULL) {
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return MEMORY_ALLOCATION_ERROR;
        }

        unsigned long long total_bytes = 0;
        if (build_feq_table(hfile_read, huffman_table, &total_bytes) != 0) {
            free(huffman_table);
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return MEMORY_ALLOCATION_ERROR;
        }

        huffman_node* huffman_tree_root = build_huffman_tree(huffman_table);
        if (huffman_tree_root == NULL) {
            free(huffman_table);
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return MEMORY_ALLOCATION_ERROR;
        }

        if (build_huffman_table(huffman_tree_root, huffman_table) != 0) {
            free_huffman_tree(huffman_tree_root);
            free(huffman_table);
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return MEMORY_ALLOCATION_ERROR;
        }

        struct huffman_file_header file_header;
        file_header.original_file_size = total_bytes;
        memcpy(file_header.huffman_table, huffman_table, sizeof(huffman_code) * 256);
        if (!WriteFile(hfile_write, &file_header, sizeof(file_header), &bytes_written, NULL)) {
            free_huffman_tree(huffman_tree_root);
            free(huffman_table);
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return FILE_WRITE_ERROR;
        }

        int compress_result = file_compress(hfile_read, hfile_write, huffman_table);
        free_huffman_tree(huffman_tree_root);
        free(huffman_table);
        if (compress_result != 0) {
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return compress_result;
        }
    }

    // -------------------------- Decompression Logic --------------------------
    else if (mode == DECOMPRESS) {
        struct huffman_file_header file_header;
        if (!ReadFile(hfile_read, &file_header, sizeof(file_header), &bytes_read, NULL) || bytes_read != sizeof(file_header)) {
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return FILE_BROKEN_ERROR;
        }

        huffman_node* huffman_tree_root = build_huffman_tree(file_header.huffman_table);
        if (huffman_tree_root == NULL) {
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return MEMORY_ALLOCATION_ERROR;
        }

        int decompress_result = file_decompress(hfile_read, hfile_write, huffman_tree_root, file_header.original_file_size);
        free_huffman_tree(huffman_tree_root);
        if (decompress_result != 0) {
            CloseHandle(hfile_read);
            CloseHandle(hfile_write);
            return decompress_result;
        }
    }

    CloseHandle(hfile_read);
    CloseHandle(hfile_write);
    return 0;
}