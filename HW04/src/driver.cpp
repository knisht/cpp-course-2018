#include "../include/huffman_engine.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <streambuf>

extern void decode(std::string const &sourcefile, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);
extern void read_and_enrich_file(std::string const &filename, huffman_engine &engine, std::ifstream &ifs);
extern void encode_file(std::string const &source, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs);

const std::string help_message = "Available commands:\n"
                                 "pack \"src\" [to \"dst\"] - Compresses file from \"src\", user can choose destination. By default, compresses file into \"src.huff\""
                                 "\nunpack \"src\" [to \"dst\"] - Restores file from it's compressed representation \"src\", user can choose destination. By default, decomresses file into \"src_decoded\"";

int main(int argc, char *argv[])
{
    huffman_engine engine{};
    std::ofstream ofs;
    std::ifstream ifs;
    if (argc < 3 || argc > 5 || argc == 4) {
        std::cout << "Wrong number of arguments! (" << argc << ")\n"
                  << help_message << std::endl;
    } else {
        std::string targetcommand(argv[1], strlen(argv[1]));
        std::string targetfile(argv[2], strlen(argv[2]));

        if (targetcommand == "pack") {
            std::cout << "Packing..." << std::endl;
            try {
                std::string outfile = targetfile + ".huff";
                if (argc == 5) {
                    if (std::string(argv[3], strlen(argv[3])) == "to") {
                        outfile = std::string(argv[4], strlen(argv[4]));
                    }
                }
                read_and_enrich_file(targetfile, engine, ifs);
                encode_file(targetfile, outfile, engine, ifs, ofs);
                std::cout << "Packed to " << outfile << std::endl;
            } catch (std::runtime_error e) {
                std::cerr << "Error occured." << std::endl;
                std::cerr << e.what() << std::endl;
            }
        } else if (targetcommand == "unpack") {
            std::cout << "Unpacking..." << std::endl;
            std::string outfile = targetfile + "_decoded";
            if (argc == 5) {
                if (std::string(argv[3], strlen(argv[3])) == "to") {
                    outfile = std::string(argv[4], strlen(argv[4]));
                }
            }
            try {
                decode(targetfile, outfile, engine, ifs, ofs);
                std::cout << "Unpacked to " << outfile << std::endl;
            } catch (std::runtime_error e) {
                std::cerr << "Error occured." << std::endl;
                std::cerr << e.what() << std::endl;
            }
        } else if (targetcommand == "help") {
            std::cout << help_message << std::endl;
        } else {
            std::cout << "Unsupported command \"" << targetcommand << "\"\n"
                      << help_message << std::endl;
        }
    }

    return 0;
}
