#include "RandW.h"

int build_feq_table(HANDLE hfile, huffman_code table[256], unsigned long long* total_bytes) {
    DWORD bytes_read = 0;
    unsigned long long total_bytes_local = 0;
    UINT8* buffer = (UINT8*)malloc(BLOCK_SIZE);

    for (int i = 0; i < 256; i++) {
        table[i].freq = 0;
    }

    do {
        if (!ReadFile(hfile, buffer, BLOCK_SIZE, &bytes_read, NULL)) {
            free(buffer);
            return FILE_ACCESS_ERROR;
        }
        for (DWORD i = 0; i < bytes_read; i++) {
            table[buffer[i]].freq++;
        }
        total_bytes_local += bytes_read;
    } while (bytes_read > 0);

    SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
    *total_bytes = total_bytes_local;
    free(buffer);
    return 0;
}

huffman_node* build_huffman_tree(huffman_code table[256]) {
    huffman_node* node_list[256];
    int node_count = 0;

    for (int i = 0; i < 256; i++) {
        if (table[i].freq > 0) {
            huffman_node* new_node = (huffman_node*)malloc(sizeof(huffman_node));
            if (!new_node) return NULL;
            new_node->weight = table[i].freq;
            new_node->ch = (unsigned char)i;
            new_node->nodetype = LEAF_NODE;
            new_node->left = NULL;
            new_node->right = NULL;
            node_list[node_count++] = new_node;
        }
    }

    if (node_count == 0) {
        return NULL;
    }
    if (node_count == 1) {
        huffman_node* root = (huffman_node*)malloc(sizeof(huffman_node));
        if (!root) return NULL;
        root->weight = node_list[0]->weight;
        root->nodetype = TREE_NODE;
        root->left = node_list[0];
        root->right = NULL;
        return root;
    }

    while (node_count > 1) {
        int min1_idx = -1, min2_idx = -1;
        for (int i = 0; i < node_count; i++) {
            if (min1_idx == -1 || node_list[i]->weight < node_list[min1_idx]->weight) {
                min2_idx = min1_idx;
                min1_idx = i;
            } else if (min2_idx == -1 || node_list[i]->weight < node_list[min2_idx]->weight) {
                min2_idx = i;
            }
        }

        huffman_node* node1 = node_list[min1_idx];
        huffman_node* node2 = node_list[min2_idx];

        huffman_node* parent_node = (huffman_node*)malloc(sizeof(huffman_node));
        if (!parent_node) return NULL;

        parent_node->weight = node1->weight + node2->weight;
        parent_node->nodetype = TREE_NODE;
        parent_node->left = node1;
        parent_node->right = node2;
        parent_node->ch = 0;

        node_list[min1_idx] = parent_node;
        if (min2_idx != node_count - 1) {
            node_list[min2_idx] = node_list[node_count - 1];
        }
        node_count--;
    }

    return node_list[0];
}

void free_huffman_tree(huffman_node* root) {
    if (root == NULL) {
        return;
    }
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

void generate_codes(huffman_node* node, huffman_code table[256], unsigned char* code, int depth) {
    if (node->nodetype == LEAF_NODE) {
        table[node->ch].length = depth;
        memcpy(table[node->ch].code, code, (depth + 7) / 8);
    } else {
        if (node->left != NULL) {
            code[depth / 8] &= ~(1 << (7 - (depth % 8)));
            generate_codes(node->left, table, code, depth + 1);
        }
        if (node->right != NULL) {
            code[depth / 8] |= (1 << (7 - (depth % 8)));
            generate_codes(node->right, table, code, depth + 1);
        }
    }
}

int build_huffman_table(huffman_node* root, huffman_code table[256]){
    for (int i = 0; i < 256; i++) {
        table[i].length = 0;
        memset(table[i].code, 0, 256 / 8);
    }

    if (root == NULL) {
        return -1;
    }

    if (root->left != NULL && root->right == NULL && root->left->nodetype == LEAF_NODE) {
        table[root->left->ch].code[0] = 0;
        table[root->left->ch].length = 1;
        return 0;
    }

    unsigned char code[256 / 8] = {0};
    generate_codes(root, table, code, 0);

    return 0;
}