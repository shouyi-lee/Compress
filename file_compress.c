#include "RandW.h"

int file_compress(HANDLE hfile_read, HANDLE hfile_write, huffman_code huffman_table[256]) {
    UINT8* in_buffer = (UINT8*)malloc(BLOCK_SIZE);
    int in_buffer_flag = 0;
    int in_buffer_index = 0;
    UINT8* out_buffer = (UINT8*)malloc(BLOCK_SIZE * 2);
    int out_buffer_flag = 0;
    int out_buffer_index = 0;
    DWORD bytes_read = 0;
    DWORD bytes_written = 0;
    UINT8* bit_buffer = (UINT8*)malloc(MAX_HUFFMAN_CODE_SIZE * 2 / BYTESIZE + 1);
    int bit_buffer_flag = 0;
    int bit_buffer_index = 0;
    UINT64 bit_buffer_q = 0;
    int bit_buffer_q_flag = 0;
    int bit_buffer_q_index = 0;

    for (;;) {
        while (bit_buffer_flag < 64) {
            if (in_buffer_flag == 0) {
                if (!ReadFile(hfile_read, in_buffer, BLOCK_SIZE, &bytes_read, NULL)) {
                    return FILE_ACCESS_ERROR;
                } else if (bytes_read == 0)
                {
                    goto end;
                }
                in_buffer_index = 0;
                in_buffer_flag = bytes_read;
            }

            unsigned char ch = in_buffer[in_buffer_index++];
            in_buffer_flag--;

            UINT8* code = huffman_table[ch].code;
            int code_length = huffman_table[ch].length;
            int tem = bit_buffer_flag + bit_buffer_index;
            for (int i = 0; i < code_length; i++) {
                if ((code[i / 8] >> (7 - (i % 8))) & 1) {
                    bit_buffer[tem / 8] |= (1 << (7 - (tem % 8)));
                } else {
                    bit_buffer[tem / 8] &= ~(1 << (7 - (tem % 8)));
                }
                tem++;
            }
            bit_buffer_flag += code_length;
        }

        for (int i = 0; i < 8; i++) {
            unsigned char byte = bit_buffer[bit_buffer_index / 8];
            byte <<= bit_buffer_index % 8;
            byte |= bit_buffer[(bit_buffer_index / 8) + 1] >> (8 - (bit_buffer_index % 8));
            bit_buffer_q |= ((UINT64)byte) << (8 * (7 - i));
        }
        bit_buffer_index += 64;
        bit_buffer_flag -= 64;

        *(UINT64*)(&out_buffer[out_buffer_index]) = bit_buffer_q;
        out_buffer_index += 8;
        bit_buffer_q = 0;
        
        if (out_buffer_index >= BLOCK_SIZE) {
            if (!WriteFile(hfile_write, out_buffer, out_buffer_index, &bytes_written, NULL)) {
                return FILE_ACCESS_ERROR;
            }
            out_buffer_index = 0;
        }

        if (bit_buffer_index + bit_buffer_flag >= MAX_HUFFMAN_CODE_SIZE) {
            int shift_amount = bit_buffer_index / 8;
            for (int i = 0; i < (bit_buffer_flag + 7) / 8; i++) {
                bit_buffer[i] = bit_buffer[i + shift_amount];
            }
            bit_buffer_index -= shift_amount * 8;
        }
    }

    end:
    for (int i = 0; i < 8; i++) {
            unsigned char byte = bit_buffer[bit_buffer_index / 8];
            byte <<= bit_buffer_index % 8;
            byte |= bit_buffer[(bit_buffer_index / 8) + 1] >> (8 - (bit_buffer_index % 8));
            bit_buffer_q |= ((UINT64)byte) << (8 * (7 - i));
        }
    *(UINT64*)(&out_buffer[out_buffer_index]) = bit_buffer_q;
    if (out_buffer_index >= BLOCK_SIZE) {
            if (!WriteFile(hfile_write, out_buffer, out_buffer_index, &bytes_written, NULL)) {
                return FILE_ACCESS_ERROR;
            }
            out_buffer_index = 0;
        }
    if (!WriteFile(hfile_write, out_buffer, out_buffer_index + (bit_buffer_flag + 7) / 8, &bytes_written, NULL)) {
                return FILE_ACCESS_ERROR;
        }

    free(in_buffer);
    free(out_buffer);
    free(bit_buffer);
    return 0;
}

