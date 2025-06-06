#ifndef LZ77COMPRESSOR_H
#define LZ77COMPRESSOR_H

#include <vector>
#include <stdint.h>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>
#include <cstring>
#include "Huffman/ac_bitstream.h"

struct SearchNode {
    std::string substring;
    size_t deslocamento; // <-- CORREÇÃO: Deslocamento pode ser grande
    SearchNode *left, *right;

    SearchNode(const std::string& s, size_t d) : substring(s), deslocamento(d), left(nullptr), right(nullptr) {}
};

class SearchTree {
public:
    // <-- CORREÇÃO: Receber tamanho como size_t
    SearchTree(size_t L);
    ~SearchTree();

    // <-- CORREÇÃO: Parâmetros atualizados para size_t
    void inserir(const std::vector<uint8_t>& dados, size_t pos, size_t posAtual);
    void remover(const std::vector<uint8_t>& dados, size_t pos, size_t posAtual);
    void buscarMelhorCasamento(const std::vector<uint8_t>& lookahead, size_t& offset, size_t& length, size_t pos) const;

private:
    SearchNode* raiz;
    size_t maxLength; // <-- CORREÇÃO: Armazenar como size_t

    void destruirArvore(SearchNode* no);
    // <-- CORREÇÃO: Parâmetros atualizados para size_t
    void inserir(SearchNode*& no, const std::string& substring, size_t deslocamento);
    void remover(SearchNode*& no, const std::string& substring);
    void buscar(SearchNode* no, const std::string& lookahead, size_t& melhorOffset, size_t& melhorTamanho, size_t pos) const;
};


class LZ77Compressor {

private:
    Bitstream bs;
    std::string inputFileName;
    std::string outputFileName;
    std::vector<uint8_t> dados;
    uint16_t tamSearch;
    uint16_t tamLookahead;
    std::vector<uint8_t> lerArquivoBytes();
    std::string nomeTemporario;

    void writeFile(std::vector<std::tuple<uint16_t, uint16_t, uint8_t>> triplas);

    void huffman();

    void debugTriplas(const std::vector<std::tuple<uint16_t, uint16_t, uint8_t>>& triplas) const;

public:
    LZ77Compressor(const std::string& inputFile, const std::string& outputFile,
                   size_t tamSearchBuffer, size_t tamLookaheadBuffer);
    void compress();
};


#endif //LZ77COMPRESSOR_H