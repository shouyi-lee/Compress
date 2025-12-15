Windows压缩和解压程序

压缩和解压文件的工具，使用哈夫曼编码算法实现数据压缩。

压缩的格式包括文件头```struct{ unsigned long long original_size; huffman_code huffman_table[256]; }```，后接压缩数据流。
