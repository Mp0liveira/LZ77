#include <iostream>
#include "LZ77Compressor.h"
#include "LZ77Descompressor.h"
#include "Huffman/HuffmanCompressor.h"

int main() {

    LZ77Compressor LZcompressor("teste.txt", "teste_compress", 4096, 16384);
    LZcompressor.compress();

    LZ77Descompressor LZdescompressor("teste_compress.bin", "teste_descompress.txt");
    LZdescompressor.decompress();
    return 0;
}
