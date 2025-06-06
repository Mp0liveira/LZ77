//
// Created by marco on 30/04/2025.
//

#include "HuffmanCompressor.h"

HuffmanCompressor::HuffmanCompressor(const std::string& inputFile, const std::string& outputFile)
    : inputFileName(inputFile), outputFileName(outputFile) {}

std::vector<uint8_t> HuffmanCompressor::lerArquivoBytes() {

    std::ifstream arquivo("../ArquivosParaComprimir/" + inputFileName, std::ios::binary);
    std::vector<uint8_t> bytes;

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << inputFileName << std::endl;
        return bytes;
    }

    uint8_t byte;
    while (arquivo.read(reinterpret_cast<char*>(&byte), 1)) {
        bytes.push_back(byte);
    }

    return bytes;
}

void HuffmanCompressor::returnCodeLenghts(Node* root, int profundidade) {
    if (!root) return;

    if (!root->left && !root->right) {
        codeLengths[root->symbol] = profundidade;
        return;
    }

    returnCodeLenghts(root->left, profundidade + 1);
    returnCodeLenghts(root->right, profundidade + 1);
}


void HuffmanCompressor::returnCanonical(const std::vector<std::pair<uint8_t, int>>& sortedSymbols) {

    uint32_t code = 0;
    int prevLen = 0;

    for (const auto& [symbol, length] : sortedSymbols) {
        // Shift à esquerda se o comprimento aumentou
        code <<= (length - prevLen);

        // Converte para string binária com zeros à esquerda
        std::string binaryCode = std::bitset<32>(code).to_string().substr(32 - length);

        canonicalCodes[symbol] = binaryCode;
        code++;        // próximo código binário
        prevLen = length;
    }

}
std::string HuffmanCompressor::byteToBinary(uint8_t byte) {
    std::string result;
    for (int i = 7; i >= 0; --i) {
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}


void HuffmanCompressor::headerFromLengths() {
    // Quantidade de símbolos distintos
    uint16_t numberSymbols = static_cast<uint16_t>(codeLengths.size());

    // 16 bits para o número de símbolos
    header += byteToBinary(numberSymbols >> 8);       // byte mais significativo
    header += byteToBinary(numberSymbols & 0xFF);     // byte menos significativo

    // Símbolo (8 bits) + tamanho do código (8 bits)
    for (const auto& [symbol, length] : codeLengths) {
        header += byteToBinary(symbol);
        header += byteToBinary(static_cast<uint8_t>(length));
    }

    // Adiciona o tamanho original do arquivo como 32 bits (4 bytes)
    uint32_t originalSize = dados.size();
    for (int i = 24; i >= 0; i -= 8) {
        uint8_t byte = (originalSize >> i) & 0xFF;
        header += byteToBinary(byte);
    }
}

// Função que calcula a entropia da fonte
float HuffmanCompressor::entropy() {
    float entropy = 0;
    float total = 0;

    for (auto it : freqMap) {
        total += it.second;
    }

    for (auto it : freqMap) {
        float p = (static_cast<float>(it.second)) / total;
        entropy += p * log2(p);
    }

    entropy = -entropy;
    return entropy;
}

// Função que calcula a média de bits por símbolo
float HuffmanCompressor::meanBits() {
    float total = 0;
    // Calcula o total de símbolos
    for (const auto& [ch, freq] : freqMap) {
        total += freq;
    }

    float bits = 0;
    // Calcula a média por símbolo e soma, resultando na média final
    for (const auto& [ch, code] : canonicalCodes) {
        float p = static_cast<float>(freqMap.at(ch)) / total;
        bits += p * code.length();
    }

    return bits;
}

void HuffmanCompressor::writeFile() {

    // Escrevendo o conteúdo codificado
    for (char c : compressed) {
        if (c == '0') {
            bs.writeBit(false); // Escreve 0
        } else {
            bs.writeBit(true); // Escreve 1
        }
    }

    bs.flushesToFile("../ArquivosComprimidos/" + outputFileName + ".bin");
}

void HuffmanCompressor::compress() {
    dados = lerArquivoBytes();

    for (uint8_t simbolo : dados) {
        freqMap[simbolo]++;
    }

    std::cout << "Entropia: " << entropy() << "\n";

    // Constrói a árvore de Huffman como base para os comprimentos
    Node* raiz = buildHuffmanTree(freqMap);

    returnCodeLenghts(raiz, 0);

    std::vector<std::pair<uint8_t, int>> sortedSymbols(codeLengths.begin(), codeLengths.end());

    // Função sort que ordena de acordo com o tamanho dos elementos e desempate por símbolo (valor do byte)
    std::sort(sortedSymbols.begin(), sortedSymbols.end(),
    [](const std::pair<uint8_t, int>& a, const std::pair<uint8_t, int>& b) {
        if (a.second != b.second)
            return a.second < b.second;
        return a.first < b.first;
    });

    returnCanonical(sortedSymbols);

    // Descomente se quiser ver a codificação de cada símbolo
    /*for (const auto& [simbolo, codeword] : canonicalCodes) {
        std::cout << simbolo << ": " << codeword << "\n";
    }*/

    for(uint8_t simbolo : dados) {
        // Lendo os bytes do arquivo e atualizando a string com a nova codificação
        compressed += canonicalCodes[simbolo];
    }

    headerFromLengths();

    std::cout << "Tamanho da compressao sem o header: " << compressed.size() << " (bits) \n";

    compressed = header + compressed;

    std::cout << "Tamanho da compressao completa (header + conteudo): " << compressed.size() << " (bits) \n";
    std::cout << "Tamanho medio compressao : " << meanBits() << " (bits/simbolo) \n";


    writeFile();
}
