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

    struct Huffman_tree {
        Node root;
        Node storage[512];
        bool empty = true;
        inline Node &operator[](size_t index) { return storage[index]; }
    };

public:
    huffman_engine();

    void add_all(const std::string &str);
    Huffman_tree generate_code();
    void set_tree(const Huffman_tree &);

    void encode(const std::string &str, bitstring &target);
    std::string decode(bitstring &source, size_t length = 0);

    void get_dictionary_representation(bitstring &order, bitstring &leaves);

    Huffman_tree decode_dictionary(bitstring const &order, bitstring const &leaves);

    size_t getlen();
    void flush();

private:
    codemap code;
    frequency_map frequencies;
    Huffman_tree tree;

    void tree_bypass(bitstring &, bitstring &, Node const &vertex, bool is_root);
};

#endif // HUFFMAN_TREE_H
