#ifndef HUFFMAN_BITSTRING_H
#define HUFFMAN_BITSTRING_H

#include <cstddef>
#include <cstdint>
#include <string>

struct bitstring {
    bitstring();

    bitstring(bitstring const &);
    bitstring &operator=(bitstring const &);

    ~bitstring();

    void append(bool symbol);
    void append(uint8_t word);
    void append(bitstring const &);
    void detach();
    std::string &data();
    size_t length() const;
    bool operator[](size_t index) const;
    char get_char(size_t index) const;
    std::string split_string();

private:
    size_t _length;
    std::string storage;
};

#endif //HUFFMAN_BITSTRING_H
