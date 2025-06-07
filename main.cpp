#include <iostream>
#include "LZ77Compressor.h"
#include "LZ77Descompressor.h"
#include "Huffman/HuffmanCompressor.h"

int main() {

    LZ77Compressor LZcompressor("dom_casmurro.txt", "dom_casmurro_compress", 4096, 16384);
    LZcompressor.compress();

    LZ77Descompressor LZdescompressor("dom_casmurro_compress.bin", "dom_casmurro_descompress.txt");
    LZdescompressor.decompress();
    return 0;
}
