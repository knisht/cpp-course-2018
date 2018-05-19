#include "../include/huffman_engine.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <streambuf>

extern void decode(std::string const &sourcefile, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);
extern void read_and_enrich_file(std::string const &filename, huffman_engine &engine, std::ifstream &ifs);
extern void encode_file(std::string const &source, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);

int main(int argc, char *argv[])
{
    huffman_engine engine{};
    if (argc < 3 || argc > 3) {
        std::cout << "Wrong number of arguments! (" << argc << ")" << std::endl;
    } else {
        std::string targetcommand(argv[1], strlen(argv[1]));
        std::cout << targetcommand << std::endl;
        std::string targetfile(argv[2], strlen(argv[2]));
        std::cout << targetfile << std::endl;

        std::ofstream ofs;
        std::ifstream ifs;
        if (targetcommand == "pack") {
            std::cout << "Packing..." << std::endl;
            try {
                read_and_enrich_file(targetfile, engine, ifs);
                encode_file(targetfile, targetfile + ".huff", engine, ifs, ofs);
                std::cout << "Packed to " << targetfile << ".huff" << std::endl;
            } catch (std::runtime_error e) {
                std::cerr << e.what() << std::endl;
            }
        } else if (targetcommand == "unpack") {
            std::cout << "Unpacking..." << std::endl;
            decode(targetfile + ".huff", targetfile, engine, ifs, ofs);
            std::cout << "Unpacked to " << targetfile << std::endl;
        } else {
            std::cout << "Unsupported command!" << std::endl;
        }
    }

    return 0;
}
