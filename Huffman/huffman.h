#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <cstdint>
#include <queue>
#include <map>


struct Node {
    uint8_t symbol;           // Agora sempre válido em folhas
    int freq;
    Node* left;
    Node* right;

    // Construtor para folha
    Node(uint8_t s, int f) : symbol(s), freq(f), left(nullptr), right(nullptr) {}

    // Construtor para nó interno
    Node(Node* l, Node* r) : symbol(0), freq(l->freq + r->freq), left(l), right(r) {}
};


struct Compare {
    bool operator()(Node* a, Node* b);
};

// Função para construir a árvore (ex: com mapa de frequências)
Node* buildHuffmanTree(const std::map<uint8_t, int>& freqs);

#endif //HUFFMAN_H
