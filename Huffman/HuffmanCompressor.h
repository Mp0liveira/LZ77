#ifndef HUFFMANCOMPRESSOR_H
#define HUFFMANCOMPRESSOR_H
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "huffman.h"
#include <bitset>
#include "ac_bitstream.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

class HuffmanCompressor {

private:
    std::string inputFileName;
    std::string outputFileName;
    std::vector<uint8_t> dados;
    std::map<uint8_t, int> freqMap;
    std::map<uint8_t, int> codeLengths;
    std::vector<std::pair<uint8_t, int>> sortedSymbols;
    std::map<uint8_t, std::string> canonicalCodes;
    std::string compressed;
    std::string header;
    Bitstream bs;

    std::vector<uint8_t> lerArquivoBytes();
    void returnCodeLenghts(Node* root, int profundidade);
    void returnCanonical(const std::vector<std::pair<uint8_t, int>>& sortedSymbols);
    std::string byteToBinary(uint8_t byte);
    void headerFromLengths();
    float entropy();
    float meanBits();
    void writeFile();

public:
    HuffmanCompressor(const std::string& inputFile, const std::string& outputFile);
    void compress();

};





#endif //HUFFMANCOMPRESSOR_H
