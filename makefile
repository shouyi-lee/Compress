all:
	gcc build_huffman_tree.c file_compress.c file_decompress.c handle_file.c RandW.h UI.c -o yasua.exe -lcomdlg32 -O3 -mwindows