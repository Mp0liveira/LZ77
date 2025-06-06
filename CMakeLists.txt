cmake_minimum_required(VERSION 3.29)
project(LZ77)

set(CMAKE_CXX_STANDARD 20)

add_executable(LZ77 main.cpp
        Huffman/huffman.h
        Huffman/huffman.cpp
        Huffman/ac_bitstream.h
        Huffman/ac_bitstream.cpp
        Huffman/HuffmanCompressor.cpp
        Huffman/HuffmanCompressor.h
        Huffman/HuffmanDescompressor.cpp
        Huffman/HuffmanDescompressor.h
        LZ77Compressor.cpp
        LZ77Compressor.h
        LZ77Descompressor.cpp
        LZ77Descompressor.h
)
