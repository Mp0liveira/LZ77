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
        // A cópia só acontece se o comprimento for maior que 0
        if (length > 0) {
            // Verifica se o offset é válido para evitar acessos fora dos limites
            if (offset > 0 && offset <= dadosDescomprimidos.size()) {

                // 1. Determina a posição inicial da cópia ANTES do laço. Ela não vai mais mudar.
                size_t start_pos = dadosDescomprimidos.size() - offset;

                // 2. O laço agora lê da posição inicial fixa + o incremento 'j'.
                for (int j = 0; j < length; ++j) {
                    uint8_t byte_a_copiar = dadosDescomprimidos[start_pos + j];
                    dadosDescomprimidos.push_back(byte_a_copiar);
                }
            }
        }
        // O símbolo é adicionado no final, após a cópia (se houver).
        dadosDescomprimidos.push_back(simbolo);
    }

    writeFile(dadosDescomprimidos);
    /*std::cout << "descomprimidos: " << std::endl;
    for (int i = 0; i < dadosDescomprimidos.size(); ++i) {
        std::cout << dadosDescomprimidos[i];
    }*/
}
