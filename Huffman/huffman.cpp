#include "huffman.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <bitset>
#include <algorithm>
#include <cmath>
#include "ac_bitstream.h"


bool Compare::operator()(Node* a, Node* b) {
    return a->freq > b->freq;
}

Node* buildHuffmanTree(const std::map<uint8_t, int>& freqs) {
    std::priority_queue<Node*, std::vector<Node*>, Compare> minHeap;

    for (const auto& pair : freqs) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    while (minHeap.size() > 1) {
        Node* left = minHeap.top(); minHeap.pop();
        Node* right = minHeap.top(); minHeap.pop();

        Node* merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        minHeap.push(merged);
    }

    return minHeap.top();
}

/*void printCodes(Node* root, const std::string& code = "") {
    if (!root) return;

    if (!root->left && !root->right && root->ch != '\0') {
        std::cout << root->ch << ": " << code << "\n";
    }

    printCodes(root->left, code + "0");
    printCodes(root->right, code + "1");
}
*/