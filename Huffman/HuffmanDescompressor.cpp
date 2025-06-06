//
// Created by marco on 01/05/2025.
//

#include "HuffmanDescompressor.h"

#include <iostream>
#include <ostream>
#include <valarray>

HuffmanDescompressor::HuffmanDescompressor(const std::string& inputFile, const std::string& outputFile)
    : inputFileName(inputFile), outputFileName(outputFile) {}

void HuffmanDescompressor::arrayBits(Bitstream& bs) {
    uint64_t size = bs.totalSize();

    for (uint64_t i = 0; i < size; i++) {
        bool bit = bs.readBit();
        inputBuffer += bit ? '1' : '0';
    }
}

void HuffmanDescompressor::descompress() {
    Bitstream bs(inputFileName);
    arrayBits(bs);
    readHeader();
    fillLUT();
    decoder();
}

void HuffmanDescompressor::readHeader() {
    // Lê os dois primeiros bytes para obter o número de símbolos (16 bits)
    std::string symbolCountBits = inputBuffer.substr(0, 16);
    uint16_t numberSymbols = static_cast<uint16_t>(std::stoi(symbolCountBits, nullptr, 2));

    // Cada símbolo ocupa 16 bits (8 bits do valor + 8 bits do comprimento)
    int headerSymbolsSize = numberSymbols * 16;

    // O campo de tamanho original tem 32 bits (4 bytes)
    int startOriginalSize = 16 + headerSymbolsSize;
    startFile = startOriginalSize + 32; // início dos dados comprimidos

    // Lê os pares símbolo/comprimento
    for (int i = 16; i < 16 + headerSymbolsSize; i += 16) {
        std::string symbolBits = inputBuffer.substr(i, 8);
        std::string lengthBits = inputBuffer.substr(i + 8, 8);

        uint8_t symbol = static_cast<uint8_t>(std::stoi(symbolBits, nullptr, 2));
        int length = std::stoi(lengthBits, nullptr, 2);

        symbolsAndLengths.push_back({symbol, length});
    }

    // Lê os 32 bits do tamanho original do arquivo (em bytes)
    std::string sizeBits = inputBuffer.substr(startOriginalSize, 32);
    originalSize = 0;

    for (int i = 0; i < 4; ++i) {
        std::string byteStr = sizeBits.substr(i * 8, 8);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 2));
        originalSize = (originalSize << 8) | byte;
    }

}



void HuffmanDescompressor::fillLUT() {
    int size = symbolsAndLengths.size();

    // Encontrando lmax
    for (int i = 0; i < size; i++) {
        lmax = std::max(lmax, symbolsAndLengths[i].second);
    }

    // Realocando o vetor da look up table
    table.resize(1 << lmax);

    // Ordenando os símbolos por tamanho
    std::sort(symbolsAndLengths.begin(), symbolsAndLengths.end(), [](auto& a, auto& b) {
    return (a.second != b.second) ? a.second < b.second : a.first < b.first;});

    uint32_t code = 0;
    int prevLen = 0;

    for (const auto& [symbol, length] : symbolsAndLengths) {
        code <<= (length - prevLen); // Shift no código

        int numFill = 1 << (lmax - length);
        for (int i = 0; i < numFill; ++i) {
            int index = (code << (lmax - length)) | i;  // Preenchendo os "don't cares"
            table[index] = {symbol, length};
        }

        code++;
        prevLen = length;
    }

}

void HuffmanDescompressor::decoder() {
    std::vector<uint8_t> contents;
    int i = startFile;

    while (i < inputBuffer.size() && contents.size() < originalSize) {
        int remaining = inputBuffer.size() - i;
        int bitsToRead = (remaining >= lmax) ? lmax : remaining;

        std::string symbolBits = inputBuffer.substr(i, bitsToRead);

        // Preenche com zeros à direita para formar um índice de lmax bits
        while (symbolBits.length() < lmax) {
            symbolBits += '0';
        }

        int index = std::stoi(symbolBits, nullptr, 2);
        LUTEntry entry = table[index];

        contents.push_back(entry.symbol);
        i += entry.codeLength;
    }

    writeFile(contents);
}


void HuffmanDescompressor::writeFile(const std::vector<uint8_t>& contents) {
    Bitstream bs2;

    // Escrevendo o conteúdo codificado
    for (uint8_t symbol : contents) {
        for (int i = 7; i >= 0; i--) {
            bool bit = (symbol >> i) & 0x1;
            bs2.writeBit(bit);
        }
    }

    bs2.flushRawToFile("../DescompressaoIntermediariaLZ77/" + outputFileName);
}