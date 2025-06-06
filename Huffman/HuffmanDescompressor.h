#ifndef HUFFMANDESCOMPRESSOR_H
#define HUFFMANDESCOMPRESSOR_H
#include <cstdint>
#include <string>
#include "ac_bitstream.h"
#include <fstream>

struct LUTEntry {
    uint8_t symbol;
    int codeLength;
};

class HuffmanDescompressor {

private:
    std::string inputFileName;
    std::string outputFileName;
    std::string inputBuffer;
    int startFile = 0;
    int lmax = 0;
    std::vector<std::pair<uint8_t, int>> symbolsAndLengths;
    std::vector<LUTEntry> table;
    uint32_t originalSize = 0;
    void fillLUT();
    void arrayBits(Bitstream& bs);
    void readHeader();
    void decoder();
    void writeFile(const std::vector<uint8_t>& contents);

public:
    HuffmanDescompressor(const std::string& inputFile, const std::string& outputFile);
    void descompress();
};

#endif //HUFFMANDESCOMPRESSOR_H
