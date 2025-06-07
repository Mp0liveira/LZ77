#include "LZ77Compressor.h"
#include "Huffman/HuffmanCompressor.h"

SearchTree::SearchTree(size_t L) : raiz(nullptr), maxLength(L) {}
SearchTree::~SearchTree() { destruirArvore(raiz); }

void SearchTree::destruirArvore(SearchNode* no) {
    if (!no) return;
    destruirArvore(no->left);
    destruirArvore(no->right);
    delete no;
}

void SearchTree::inserir(const std::vector<uint8_t>& dados, size_t pos) {
    if (pos >= dados.size()) return;

    // Calcula quantos bytes podemos ler com segurança
    size_t count = std::min(maxLength, dados.size() - pos);
    if (count == 0) return;

    std::string substring(reinterpret_cast<const char*>(dados.data() + pos), count);
    inserir(raiz, substring, pos);
}

void SearchTree::inserir(SearchNode*& no, const std::string& substring, size_t pos) {
    if (!no) {
        no = new SearchNode(substring, pos);
        return;
    }
    // Evita inserir duplicatas exatas da mesma posição
    if (substring == no->substring && pos == no->position) {
        return;
    }
    if (substring < no->substring)
        inserir(no->left, substring, pos);
    else
        inserir(no->right, substring, pos);
}

void SearchTree::remover(const std::vector<uint8_t>& dados, size_t pos) {
    if (pos >= dados.size()) return;

    size_t count = std::min(maxLength, dados.size() - pos);
    if (count == 0) return;

    std::string substring(reinterpret_cast<const char*>(dados.data() + pos), count);
    remover(raiz, substring);
}

void SearchTree::buscarMelhorCasamento(const std::vector<uint8_t>& lookahead, size_t& offset, size_t& length, size_t current_pos) const {
    if (lookahead.empty()) {
        offset = 0;
        length = 0;
        return;
    }
    std::string str(lookahead.begin(), lookahead.end());
    offset = 0;
    length = 0;
    buscar(raiz, str, offset, length, current_pos);
}

void SearchTree::buscar(SearchNode* no, const std::string& lookahead, size_t& melhorOffset, size_t& melhorTamanho, size_t current_pos) const {
    if (!no) return;

    if (lookahead < no->substring) {
        buscar(no->left, lookahead, melhorOffset, melhorTamanho, current_pos);
    } else {
        buscar(no->right, lookahead, melhorOffset, melhorTamanho, current_pos);
    }

    // Compara com o nó atual após verificar os filhos (encontra o mais próximo primeiro)
    size_t tamanho = 0;
    size_t maxComp = std::min(lookahead.size(), no->substring.size());

    for (size_t i = 0; i < maxComp; ++i) {
        if (lookahead[i] == no->substring[i])
            ++tamanho;
        else
            break;
    }

    // Aceita o match APENAS se ele estiver no passado (no->position < current_pos)
    // E se o comprimento for melhor que o já encontrado.
    if (tamanho > melhorTamanho && no->position < current_pos) {
        melhorTamanho = tamanho;
        melhorOffset = current_pos - no->position;
    }
}

// A função de remover nós da árvore permanece a mesma
void SearchTree::remover(SearchNode*& no, const std::string& substring) {
    if (!no) return;

    if (substring < no->substring) {
        remover(no->left, substring);
    } else if (substring > no->substring) {
        remover(no->right, substring);
    } else { // Encontrou a substring
        // Nó sem filhos ou com um filho
        if (no->left == nullptr) {
            SearchNode* temp = no->right;
            delete no;
            no = temp;
        } else if (no->right == nullptr) {
            SearchNode* temp = no->left;
            delete no;
            no = temp;
        } else { // Nó com dois filhos
            SearchNode* temp = no->right;
            while (temp->left != nullptr) temp = temp->left; // Encontra o sucessor in-order

            // Copia o conteúdo do sucessor para este nó
            no->substring = temp->substring;
            no->position = temp->position;

            // Remove o sucessor que foi copiado
            remover(no->right, temp->substring);
        }
    }
}

LZ77Compressor::LZ77Compressor(const std::string& inputFile, const std::string& outputFile,
                               size_t tamSearchBuffer, size_t tamLookaheadBuffer)
    : inputFileName(inputFile), outputFileName(outputFile),
      tamSearch(tamSearchBuffer), tamLookahead(tamLookaheadBuffer) {}

std::vector<uint8_t> LZ77Compressor::lerArquivoBytes() {
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

void LZ77Compressor::writeFile(std::vector<std::tuple<uint16_t, uint16_t, uint8_t>> triplas) {
    std::vector<int> bits;

    for (const auto& [offset, length, simbolo] : triplas) {
        for (int i = 15; i >= 0; --i) bits.push_back((offset >> i) & 1);
        for (int i = 15; i >= 0; --i) bits.push_back((length >> i) & 1);
        for (int i = 7;  i >= 0; --i) bits.push_back((simbolo >> i) & 1);
    }

    for (const auto& bit : bits) {
        if (bit) {
            bs.writeBit(true);
        } else {
            bs.writeBit(false);
        }
    }

    nomeTemporario = "../CompressaoIntermediariaLZ77/LZZtemporario" + outputFileName +".bin";
    bs.flushesToFile("../CompressaoIntermediariaLZ77/LZZtemporario" + outputFileName +".bin");
}

void LZ77Compressor::huffman() {
    HuffmanCompressor huffman(nomeTemporario, outputFileName);
    huffman.compress();
}

void LZ77Compressor::compress() {
    dados = lerArquivoBytes();
    if (dados.empty()) {
        return;
    }

    std::vector<std::tuple<uint16_t, uint16_t, uint8_t>> triplas;

    const size_t L = tamLookahead;
    const size_t S = tamSearch;

    size_t pos = 0;

    SearchTree arvore(L);

    std::cout << "=== LZ77 com Árvore de Busca e Janela Deslizante ===\n";
    std::cout << "Search Buffer: " << S << ", Lookahead Buffer: " << L << std::endl;

    //Inicialização da árvore (preencher a primeira janela)
    size_t initialFill = std::min(dados.size(), L);
    for (size_t i = 0; i < initialFill; ++i) {
        arvore.inserir(dados, i);
    }

    //Loop principal com janela deslizante (sem reconstruir a árvore)
    while (pos < dados.size()) {
        size_t melhorOffset = 0;
        size_t melhorLength = 0;

        std::vector<uint8_t> lookahead;
        lookahead.reserve(L);
        for (size_t i = 0; i < L && (pos + i) < dados.size(); ++i) {
            lookahead.push_back(dados[pos + i]);
        }

        arvore.buscarMelhorCasamento(lookahead, melhorOffset, melhorLength, pos);

        uint8_t nextChar = (pos + melhorLength < dados.size()) ? dados[pos + melhorLength] : 0;

        triplas.emplace_back(static_cast<uint16_t>(melhorOffset), static_cast<uint16_t>(melhorLength), nextChar);

        size_t passo = melhorLength + 1;

        for (size_t i = 0; i < passo && (pos + i) < dados.size(); ++i) {
            ssize_t posRemover = (ssize_t)(pos + i) - S;
            size_t posInserir = pos + i + L;

            // As chamadas agora são mais simples, sem 'pos + i'
            if (posRemover >= 0) {
                arvore.remover(dados, posRemover);
            }
            if (posInserir + L <= dados.size()) { // Garante que não insere substring parcial
                arvore.inserir(dados, posInserir);
            }
        }

        pos += passo;

        if (pos % 10240 == 0 || pos >= dados.size()) {
            std::cout << "Processado: " << pos * 100 / dados.size() << "%\r";
        }
    }

    std::cout << "\nCompressão concluída. Total de triplas: " << triplas.size() << std::endl;

    //debugTriplas(triplas);
    writeFile(triplas);
    huffman();
}

void LZ77Compressor::debugTriplas(const std::vector<std::tuple<uint16_t, uint16_t, uint8_t>>& triplas) const {
    std::cout << "=== Triplas geradas (Offset, Length, Próximo Simbolo) ===\n";
    for (size_t i = 0; i < triplas.size(); ++i) {
        const auto& [offset, length, nextChar] = triplas[i];
        std::cout << "Tripla " << i << ": <"
                  << offset << ", "
                  << length << ", "
                  << static_cast<int>(nextChar) << ">\n";
    }
    std::cout << "=========================================================\n";
}