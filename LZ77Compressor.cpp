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

// <-- CORREÇÃO: Parâmetro 'deslocamento' é size_t
void SearchTree::inserir(SearchNode*& no, const std::string& substring, size_t deslocamento) {
    if (!no) {
        no = new SearchNode(substring, deslocamento);
        return;
    }
    if (substring < no->substring)
        inserir(no->left, substring, deslocamento);
    else
        inserir(no->right, substring, deslocamento);
}

// <-- CORREÇÃO: Parâmetros e variáveis internas são size_t
void SearchTree::inserir(const std::vector<uint8_t>& dados, size_t pos, size_t posAtual) {
    if (pos + maxLength > dados.size()) return;

    std::string substring;
    substring.reserve(maxLength); // Otimização: pré-aloca memória
    for (size_t i = 0; i < maxLength; ++i) {
        if (pos + i < dados.size()) {
            substring += dados[pos + i];
        } else {
            break;
        }
    }

    if (substring.empty()) return;

    // <-- LÓGICA: O deslocamento é a distância da posição atual para a posição do match
    size_t deslocamento = posAtual - pos;
    inserir(raiz, substring, deslocamento);
}

// <-- CORREÇÃO: Parâmetros e variáveis internas são size_t
void SearchTree::remover(const std::vector<uint8_t>& dados, size_t pos, size_t posAtual) {
    if (pos + maxLength > dados.size()) return;

    std::string substring;
    substring.reserve(maxLength);
    for (size_t i = 0; i < maxLength; ++i) {
         if (pos + i < dados.size()) {
            substring += dados[pos + i];
        } else {
            break;
        }
    }
    if (substring.empty()) return;

    remover(raiz, substring);
}

// <-- LÓGICA CRÍTICA CORRIGIDA: Sem valor sentinela (-1)
void SearchTree::buscarMelhorCasamento(const std::vector<uint8_t>& lookahead, size_t& offset, size_t& length, size_t pos) const {
    std::string str;
    if (lookahead.empty()) {
        offset = 0;
        length = 0;
        return;
    }

    size_t tamanhoLookahead = std::min((size_t)lookahead.size(), maxLength);
    str.reserve(tamanhoLookahead);
    for (size_t i = 0; i < tamanhoLookahead; ++i)
        str += lookahead[i];

    // <-- CORREÇÃO: Inicializa com "nenhum match" (0,0). A função 'buscar' atualiza se encontrar algo.
    offset = 0;
    length = 0;
    buscar(raiz, str, offset, length, pos);
}


// <-- CORREÇÃO: Todos os parâmetros e variáveis de tamanho/posição são size_t
void SearchTree::buscar(SearchNode* no, const std::string& lookahead, size_t& melhorOffset, size_t& melhorTamanho, size_t pos) const {
    while (no) {
        size_t tamanho = 0;
        size_t maxComp = std::min(lookahead.size(), no->substring.size());

        for (size_t i = 0; i < maxComp; ++i) {
            if (lookahead[i] == no->substring[i])
                ++tamanho;
            else
                break;
        }

        if (tamanho > melhorTamanho) {
            melhorTamanho = tamanho;
            melhorOffset = no->deslocamento;
        }

        // Otimização: se o melhor match possível já foi encontrado para este ramo, não precisa descer mais
        if (melhorTamanho == lookahead.size()) return;

        if (lookahead < no->substring)
            no = no->left;
        else
            no = no->right;
    }
}

void SearchTree::remover(SearchNode*& no, const std::string& substring) {
    if (!no) return;

    if (substring < no->substring) {
        remover(no->left, substring);
    } else if (substring > no->substring) {
        remover(no->right, substring);
    } else {
        if (!no->left && !no->right) {
            delete no;
            no = nullptr;
        } else if (!no->left) {
            SearchNode* temp = no;
            no = no->right;
            delete temp;
        } else if (!no->right) {
            SearchNode* temp = no;
            no = no->left;
            delete temp;
        } else {
            SearchNode* sucessor = no->right;
            while (sucessor->left)
                sucessor = sucessor->left;

            no->substring = sucessor->substring;
            no->deslocamento = sucessor->deslocamento;
            remover(no->right, sucessor->substring);
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

    // <-- CORREÇÃO: Usar size_t para consistência, embora uint16_t funcionasse.
    const size_t L = tamLookahead;
    const size_t S = tamSearch;

    // <-- CORREÇÃO CRÍTICA: 'pos' deve ser size_t para evitar overflow.
    size_t pos = 0;

    SearchTree arvore(L);

    std::cout << "=== LZ77 com Árvore de Busca e Janela Deslizante ===\n";
    std::cout << "Search Buffer: " << S << ", Lookahead Buffer: " << L << std::endl;

    // <-- LÓGICA: Inicialização da árvore (preencher a primeira janela)
    size_t initialFill = std::min(dados.size(), L);
    for (size_t i = 0; i < initialFill; ++i) {
        arvore.inserir(dados, i, i);
    }

    // <-- LÓGICA: Loop principal com janela deslizante (sem reconstruir a árvore)
    while (pos < dados.size()) {
        // <-- CORREÇÃO: Tipos devem ser size_t para corresponder à função chamada.
        size_t melhorOffset = 0;
        size_t melhorLength = 0;

        std::vector<uint8_t> lookahead;
        lookahead.reserve(L);
        for (size_t i = 0; i < L && (pos + i) < dados.size(); ++i) {
            lookahead.push_back(dados[pos + i]);
        }

        arvore.buscarMelhorCasamento(lookahead, melhorOffset, melhorLength, pos);

        uint8_t nextChar = (pos + melhorLength < dados.size()) ? dados[pos + melhorLength] : 0;

        // <-- CORREÇÃO: static_cast para deixar a conversão de size_t para uint16_t explícita e clara.
        // É seguro porque sabemos que offset <= S e length <= L.
        triplas.emplace_back(static_cast<uint16_t>(melhorOffset), static_cast<uint16_t>(melhorLength), nextChar);

        size_t passo = melhorLength + 1;

        for (size_t i = 0; i < passo && (pos + i) < dados.size(); ++i) {
            // <-- CORREÇÃO CRÍTICA: Usar tipo com sinal para evitar underflow com posições iniciais.
            ssize_t posRemover = (ssize_t)(pos + i) - S;
            size_t posInserir = pos + i + L;

            if (posRemover >= 0) {
                 arvore.remover(dados, posRemover, pos + i);
            }
            if (posInserir < dados.size()) {
                 arvore.inserir(dados, posInserir, pos + i);
            }
        }

        pos += passo;

        if (pos % 10240 == 0 || pos >= dados.size()) {
            std::cout << "Processado: " << pos * 100 / dados.size() << "%\r";
        }
    }

    std::cout << "\nCompressão concluída. Total de triplas: " << triplas.size() << std::endl;


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