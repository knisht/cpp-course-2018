#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>
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
    for (size_t i = 0; i < 1000; ++i) {
        int curturn = rand() % 520;
        if (curturn < 256) {
            engine.add_all(std::string{static_cast<char>(curturn)});
            text.push_back(static_cast<char>(curturn));
        } else if (curturn < 519) {
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

TEST(correctness, big)
{
    for (size_t j = 0; j < 10; ++j) {
        std::string text = "";
        huffman_engine engine{};
        bitstring src;
        for (size_t i = 0; i < 1000000; ++i) {
            text.push_back((char)(32 + (rand() % 120)));
        }
        engine.add_all(text);
        engine.set_tree(engine.generate_code());
        engine.encode(text, src);
        EXPECT_EQ(engine.decode(src), text);
    }
}

extern void decode(std::string const &sourcefile, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);
extern void read_and_enrich_file(std::string const &filename, huffman_engine &engine, std::ifstream &ifs);
extern void encode_file(std::string const &source, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);

size_t const buffer_size = 1 << 15;
bool check_files_equality(std::string const &filepath1, std::string const &filepath2)
{
    std::ifstream lFile(filepath1.c_str(), std::ios::in | std::ios::binary);
    std::ifstream rFile(filepath2.c_str(), std::ios::in | std::ios::binary);

    if (!lFile.good() || !rFile.good()) {
        return false;
    }

    std::streamsize lReadBytesCount = 0;
    std::streamsize rReadBytesCount = 0;
    char buffer1[buffer_size];
    char buffer2[buffer_size];
    do {
        lFile.read(buffer1, buffer_size);
        rFile.read(buffer2, buffer_size);
        lReadBytesCount = lFile.gcount();
        rReadBytesCount = rFile.gcount();

        if (lReadBytesCount != rReadBytesCount || std::memcmp(buffer1, buffer2, lReadBytesCount) != 0) {
            return false;
        }
    } while (lFile.good() || rFile.good());

    return true;
}

TEST(correctness, simple_text)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test1.txt";
    std::string outfile = "../test/test1-decoded.txt";
    huffman_engine engine{};
    read_and_enrich_file(infile, engine, ifs);
    encode_file(infile, infile + "huff", engine, ifs, ofs);
    decode(infile + "huff", outfile, engine, ifs, ofs);
    ASSERT_TRUE(check_files_equality(infile, outfile));
    remove(outfile.c_str());
    remove(infile.append("huff").c_str());
}

TEST(correctness, unicode_symbols)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test2.txt";
    std::string outfile = "../test/test2-decoded.txt";
    huffman_engine engine{};
    read_and_enrich_file(infile, engine, ifs);
    encode_file(infile, infile + "huff", engine, ifs, ofs);
    decode(infile + "huff", outfile, engine, ifs, ofs);
    ASSERT_TRUE(check_files_equality(infile, outfile));
    remove(outfile.c_str());
    remove(infile.append("huff").c_str());
}

TEST(correctness, many_equal_symbols)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test3.txt";
    std::string outfile = "../test/test3-decoded.txt";
    huffman_engine engine{};
    read_and_enrich_file(infile, engine, ifs);
    encode_file(infile, infile + "huff", engine, ifs, ofs);
    decode(infile + "huff", outfile, engine, ifs, ofs);
    ASSERT_TRUE(check_files_equality(infile, outfile));
    remove(outfile.c_str());
    remove(infile.append("huff").c_str());
}

TEST(correctness, file_not_found)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test4.txt";
    std::string outfile = "../test/test4-decoded.txt";
    huffman_engine engine{};

    ASSERT_THROW(read_and_enrich_file(infile, engine, ifs), std::runtime_error);
    ASSERT_THROW(encode_file(infile, infile + "huff", engine, ifs, ofs), std::runtime_error);
    ASSERT_THROW(decode(infile + "huff", outfile, engine, ifs, ofs), std::runtime_error);
}

//Following test is too large to transfer it by net :(

TEST(correctness, file_creation)
{
    std::ofstream ofs;
    std::string infile = "../test/test5.txt";
    std::string outfile = "../test/test5-decoded.txt";
    ofs.open(infile, std::ios_base::binary);
    huffman_engine engine{};
    for (int j = 0; j < 900; ++j)
        for (size_t i = 0; i < 100000; ++i) {
            ofs << static_cast<char>(rand() % 256);
            if (i % 10000 == 0)
                ofs << std::flush;
        }
    ofs.close();
}

TEST(correctness, onlycompress)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test5.txt";
    std::string outfile = "../test/test5-decoded.txt";
    huffman_engine engine{};
    read_and_enrich_file(infile, engine, ifs);
    encode_file(infile, infile + "huff", engine, ifs, ofs);
}

TEST(correctness, onlydecode)
{
    std::ifstream ifs;
    std::ofstream ofs;
    std::string infile = "../test/test5.txt";
    std::string outfile = "../test/test5-decoded.txt";
    huffman_engine engine{};
    decode(infile + "huff", outfile, engine, ifs, ofs);
    remove(outfile.c_str());
    remove(infile.c_str());
    remove(infile.append("huff").c_str());
}
