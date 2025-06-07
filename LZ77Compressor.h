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
    size_t position;
    SearchNode *left, *right;

    SearchNode(const std::string& s, size_t pos) : substring(s), position(pos), left(nullptr), right(nullptr) {}
};

class SearchTree {
public:
    SearchTree(size_t L);
    ~SearchTree();

    void inserir(const std::vector<uint8_t>& dados, size_t pos);
    void remover(const std::vector<uint8_t>& dados, size_t pos);
    void buscarMelhorCasamento(const std::vector<uint8_t>& lookahead, size_t& offset, size_t& length, size_t current_pos) const;

private:

    SearchNode* raiz;
    size_t maxLength;
    void destruirArvore(SearchNode* no);
    void inserir(SearchNode*& no, const std::string& substring, size_t pos);
    void remover(SearchNode*& no, const std::string& substring);
    void buscar(SearchNode* no, const std::string& lookahead, size_t& melhorOffset, size_t& melhorTamanho, size_t current_pos) const;
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