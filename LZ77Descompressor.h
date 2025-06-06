#ifndef LZ77DESCOMPRESSOR_H
#define LZ77DESCOMPRESSOR_H
#include <string>
#include <vector>
#include "../Huffman/ac_bitstream.h"
#include <stdint.h>
#include "../Huffman/HuffmanDescompressor.h"


class LZ77Descompressor {

private:
    std::string inputFileName;
    std::string outputFileName;
    std::string caminhoArquivo;
    void huffman();

    void writeFile(const std::vector<uint8_t> &contents);

public:
    LZ77Descompressor(const std::string& inputFile, const std::string& outputFile);
    void decompress();
};





#endif //LZ77DESCOMPRESSOR_H
