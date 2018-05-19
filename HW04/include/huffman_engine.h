#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H
#include "bitstring.h"
#include <cstddef>
#include <map>
#include <vector>

class huffman_engine
{
    using word_type = uint8_t;
    using codemap = bitstring[512];
    using frequency_map = size_t[512];

    struct Node {
        size_t left_child;
        size_t right_child;
        size_t weight;
        word_type word;
    };

public:
    huffman_engine();
    void add(char word);
    void add_all(const std::string &str);
    void encode(const std::string &str, bitstring &target);

    std::vector<Node> generate_code();

    void set_tree(std::vector<Node>);

    std::string decode(bitstring &source, size_t length = 0);

    void get_dictionary_representation(bitstring &order, bitstring &leaves);

    std::vector<Node> decode_dictionary(const bitstring &order, const bitstring &leaves);

    unsigned long long getlen();
    void flush();

private:
    codemap code;
    frequency_map frequencies;
    std::vector<Node> tree;
    Node tree_root;

    struct NodeComparator {
        bool operator()(Node const &a, Node const &b) const
        {
            return a.weight < b.weight;
        }
    };

    void tree_bypass(bitstring &, bitstring &, Node const &vertex);
};

#endif // HUFFMAN_TREE_H
