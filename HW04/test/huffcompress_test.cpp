#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <test/gtest.h>
#include <utility>
#include <vector>

#include "../include/bitstring.h"
#include "../include/huffman_engine.h"

TEST(correctness, basic)
{
    std::string text = "Hello world!";
    huffman_engine engine{};
    bitstring src;
    engine.add_all(text);
    engine.set_tree(engine.generate_code());
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}

TEST(correctness, lot_of_letters)
{
    std::string text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    huffman_engine engine{};
    bitstring src;
    engine.add_all(text);
    engine.set_tree(engine.generate_code());
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}

TEST(correctness, empty)
{
    std::string text = "";
    huffman_engine engine{};
    bitstring src;
    engine.add_all(text);
    engine.set_tree(engine.generate_code());
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}

TEST(correctness, reuse)
{
    std::string text = "abrakadabra";
    huffman_engine engine{};
    bitstring src;
    for (size_t i = 0; i < 1000; ++i) {
        engine.add_all(text);
    }
    engine.flush();
    engine.add_all(text);
    engine.set_tree(engine.generate_code());
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}

TEST(correctness, dictionary_coding)
{
    std::string text = "abrakadabra";
    huffman_engine engine{};
    bitstring src;
    for (size_t i = 0; i < 10; ++i) {
        text += text;
    }
    engine.add_all(text);
    engine.set_tree(engine.generate_code());
    bitstring order, words;
    engine.get_dictionary_representation(order, words);
    engine.set_tree(engine.decode_dictionary(order, words));
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}

TEST(correctness, random)
{
    std::string text = "";
    huffman_engine engine{};
    bitstring src;
    for (size_t i = 0; i < 100; ++i) {
        int curturn = rand() % 600;
        if (curturn < 256) {
            engine.add(static_cast<char>(curturn));
            text.push_back(static_cast<char>(curturn));
        } else if (curturn < 512) {
            engine.set_tree(engine.generate_code());
            bitstring order, words;
            engine.get_dictionary_representation(order, words);
            engine.set_tree(engine.decode_dictionary(order, words));
        } else {
            engine.flush();
            text = "";
        }
    }
    engine.set_tree(engine.generate_code());
    engine.encode(text, src);
    EXPECT_EQ(engine.decode(src), text);
}
