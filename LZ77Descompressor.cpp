#include "LZ77Descompressor.h"

#include <iostream>

LZ77Descompressor::LZ77Descompressor(const std::string& inputFile, const std::string& outputFile)
    : inputFileName("../ArquivosComprimidos/" + inputFile), outputFileName(outputFile) {}

void LZ77Descompressor::huffman() {
    HuffmanDescompressor huffman(inputFileName, "TempDescomp" + outputFileName + ".bin");
    huffman.descompress();
    caminhoArquivo = "../DescompressaoIntermediariaLZ77/TempDescomp" + outputFileName + ".bin";
}

void LZ77Descompressor::writeFile(const std::vector<uint8_t>& contents) {
    Bitstream bs2;

    // Escrevendo o conteúdo codificado
    for (uint8_t symbol : contents) {
        for (int i = 7; i >= 0; i--) {
            bool bit = (symbol >> i) & 0x1;
            bs2.writeBit(bit);
        }
    }

    bs2.flushRawToFile("../ArquivosDescomprimidos/" + outputFileName);
}

void LZ77Descompressor::decompress() {
    huffman();
    Bitstream bs(caminhoArquivo);  // Abre para leitura de bits
    std::vector<std::tuple<uint16_t, uint16_t, uint8_t>> triplas;
    std::vector<uint8_t> dadosDescomprimidos;

    const int TAM_OFFSET = 16;
    const int TAM_LENGTH = 16;
    const int TAM_SYMBOL = 8;

    uint64_t size = bs.totalSize();
    uint64_t i = 0;

    while (i + TAM_OFFSET + TAM_LENGTH + TAM_SYMBOL <= size) {
        // Lê offset
        uint16_t offset = 0;
        for (int b = TAM_OFFSET - 1; b >= 0; --b, ++i) {
            bool bit = bs.readBit();
            offset |= (bit << b);
        }

        // Lê length
        uint16_t length = 0;
        for (int b = TAM_LENGTH - 1; b >= 0; --b, ++i) {
            bool bit = bs.readBit();
            length |= (bit << b);
        }

        // Lê símbolo
        uint8_t simbolo = 0;
        for (int b = TAM_SYMBOL - 1; b >= 0; --b, ++i) {
            bool bit = bs.readBit();
            simbolo |= (bit << b);
        }

        triplas.emplace_back(offset, length, simbolo);
    }

    // Reconstrói os dados a partir das triplas
    for (const auto& [offset, length, simbolo] : triplas) {
        for (int j = 0; j < length; ++j) {
            if (offset == 0 || dadosDescomprimidos.size() < offset) break;
            uint8_t byte = dadosDescomprimidos[dadosDescomprimidos.size() - offset];
            dadosDescomprimidos.push_back(byte);
        }
        dadosDescomprimidos.push_back(simbolo);
    }

    writeFile(dadosDescomprimidos);
    /*std::cout << "descomprimidos: " << std::endl;
    for (int i = 0; i < dadosDescomprimidos.size(); ++i) {
        std::cout << dadosDescomprimidos[i];
    }*/
}
