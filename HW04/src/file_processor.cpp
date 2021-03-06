#include "../include/huffman_engine.h"
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <streambuf>

const size_t buffer_size = 1 << 17;

void decode(std::string const &sourcefile, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs)
{
    ifs.open(sourcefile, std::ios_base::binary | std::ios_base::ate);
    long long available = ifs.tellg();

    if (available == -1)
        throw std::runtime_error("File not found!");

    uint64_t file_length = static_cast<uint64_t>(available);
    char buffer[buffer_size];

    try {
        //Dictionary reading
        bitstring dictionary_information;
        bitstring compressed;
        bitstring bypass;
        bitstring words;

        ifs.seekg(0, std::ios_base::beg);

        ifs.read(buffer, 2);
        dictionary_information.append(static_cast<uint8_t>(buffer[0]));
        dictionary_information.append(static_cast<uint8_t>(buffer[1]));

        size_t dictionary_length = 0;
        for (size_t i = 0; i < 16; ++i) {
            if (dictionary_information[i])
                dictionary_length += (1 << i);
        }

        if ((dictionary_length + 7) / 8 > file_length - 2)
            throw std::runtime_error("File corrupted");

        ifs.read(buffer, (dictionary_length + 7) / 8);
        for (size_t i = 0; i < (dictionary_length + 7) / 8; ++i)
            bypass.append(static_cast<uint8_t>(buffer[i]));

        dictionary_information = bitstring{};
        size_t delta = (((dictionary_length + 7) / 8) * 8) - dictionary_length;
        if (delta) {
            for (size_t i = 0; i < delta; ++i) {
                dictionary_information.append(bypass[dictionary_length + i]);
            }
            for (size_t i = 0; i < delta; ++i) {
                bypass.detach();
            }
        }

        size_t words_amount = 0;
        ifs.read(buffer, 2);
        dictionary_information.append(static_cast<uint8_t>(buffer[0]));
        dictionary_information.append(static_cast<uint8_t>(buffer[1]));
        for (size_t i = 0; i < 16; ++i) {
            if (dictionary_information[i])
                words_amount += (1 << i);
        }

        if (words_amount > std::min(file_length - 4 - (dictionary_length + 7) / 8, static_cast<uint64_t>(256)))
            throw std::runtime_error("File corrupted");

        ifs.read(buffer, static_cast<std::streamsize>(words_amount));
        for (size_t i = 0; i < words_amount; ++i) {
            dictionary_information.append(static_cast<uint8_t>(buffer[i]));
        }
        for (size_t i = 0; i < words_amount * 8; ++i) {
            words.append(dictionary_information[16 + i]);
        }

        size_t text_size = 0;
        ifs.read(buffer, 8);
        for (size_t i = 0; i < 8; ++i) {
            dictionary_information.append(static_cast<uint8_t>(buffer[i]));
        }
        for (uint64_t i = 0; i < 64; ++i) {
            if (dictionary_information[static_cast<size_t>(16 + i + words_amount * 8)])
                text_size += (1ull << i);
        }
        // Compressed code reading
        bitstring code_accumulator;
        for (size_t i = 80 + words_amount * 8; i < std::min(dictionary_information.length(), static_cast<size_t>(80 + (words_amount * 8) + text_size)); ++i) {
            code_accumulator.append(dictionary_information[i]);
        }

        engine.flush();
        engine.set_tree(engine.decode_dictionary(bypass, words));
        ofs.open(outfile, std::ios::binary);
        file_length -= 12 + words_amount + (dictionary_length + 7) / 8;
        text_size -= code_accumulator.length();

        if (text_size / 8 > file_length)
            throw std::runtime_error("File corrupted");

        while (file_length > buffer_size) {
            file_length -= buffer_size;
            text_size -= buffer_size * 8;
            ifs.read(buffer, buffer_size);
            for (size_t i = 0; i < buffer_size; ++i) {
                code_accumulator.append(reinterpret_cast<uint8_t &>(buffer[i]));
            }
            std::string decoded = engine.decode(code_accumulator);
            ofs << decoded << std::flush;
        }
        size_t left_for_complete = code_accumulator.length();
        ifs.read(buffer, static_cast<std::streamsize>(file_length));
        for (size_t i = 0; i < file_length; ++i) {
            code_accumulator.append(reinterpret_cast<uint8_t &>(buffer[i]));
        }
        if (text_size + left_for_complete != 0) {
            std::string decoded = engine.decode(code_accumulator, static_cast<size_t>(text_size + left_for_complete));
            ofs << decoded << std::flush;
        }
        ifs.close();
        ofs.close();
    } catch (std::runtime_error e) {
        ifs.close();
        ofs.close();
        throw;
    }
}

void read_and_enrich_file(std::string const &filename, huffman_engine &engine, std::ifstream &ifs)
{
    ifs.open(filename, std::ios_base::binary | std::ios_base::ate);

    long long available = ifs.tellg();

    if (available == -1)
        throw std::runtime_error("File not found!");

    uint64_t file_length = static_cast<uint64_t>(available);
    char buffer[buffer_size];
    ifs.seekg(0, std::ios::beg);

    try {
        while (file_length > buffer_size) {
            file_length -= buffer_size;
            ifs.read(buffer, buffer_size);
            std::string buffer_string(buffer, buffer_size);
            engine.add_all(buffer_string);
        }
        ifs.read(buffer, static_cast<std::streamsize>(file_length));
        std::string bufstr(buffer, static_cast<size_t>(file_length));
        engine.add_all(bufstr);
        engine.set_tree(engine.generate_code());
        ifs.close();
    } catch (...) {
        ifs.close();
        throw;
    }
}

void encode_file(std::string const &source, std::string const &outfile, huffman_engine &engine, std::ifstream &ifs, std::ofstream &ofs)
{
    ifs.open(source, std::ios_base::binary | std::ios_base::ate);
    long long available = ifs.tellg();

    if (available == -1)
        throw std::runtime_error("File not found!");

    uint64_t file_length = static_cast<uint64_t>(available);

    ifs.seekg(0, std::ios_base::beg);

    try {
        ofs.open(outfile, std::ios_base::binary);

        bitstring tree_representation;
        bitstring words_order;
        engine.get_dictionary_representation(tree_representation, words_order);

        size_t word_number = words_order.length() / 8;
        size_t tree_size = tree_representation.length();
        bitstring encoded_representation;
        for (size_t i = 0; i < 16; ++i) {
            encoded_representation.append(((tree_size & (1 << i)) != 0));
        }
        encoded_representation.append(tree_representation);
        for (size_t i = 0; i < 16; ++i) {
            encoded_representation.append(((word_number & (1 << i)) != 0));
        }
        encoded_representation.append(words_order);
        size_t codelen = engine.getlen();
        for (size_t i = 0; i < 64; ++i) {
            uint64_t tmp = codelen & (1ull << i);
            encoded_representation.append((tmp != 0ull));
        }

        ofs << encoded_representation.split_string() << std::flush;

        char buffer[buffer_size];
        while (file_length > buffer_size) {
            file_length -= buffer_size;

            ifs.read(buffer, buffer_size);
            std::string buffer_string(buffer, buffer_size);
            engine.encode(buffer_string, encoded_representation);
            std::string encoded_string = encoded_representation.split_string();
            ofs << encoded_string << std::flush;
        }
        ifs.read(buffer, static_cast<std::streamsize>(file_length));

        std::string bufstr(buffer, static_cast<size_t>(file_length));

        engine.encode(bufstr, encoded_representation);
        ofs << encoded_representation.split_string() << std::flush;
        if (encoded_representation.length() != 0)
            ofs << encoded_representation.data() << std::flush;
        ofs.close();
        ifs.close();
    } catch (std::runtime_error e) {
        ofs.close();
        ifs.close();
        throw;
    }
}
