all:
	gcc build_huffman_tree.c file_compress.c handle_file.c RandW.h UI.c -o yasua.exe -lcomdlg32 -mwindows