#ifndef HUFFMAN_BITSTRING_H
#define HUFFMAN_BITSTRING_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct bitstring {
    bitstring();

    bitstring(bitstring const &);
    bitstring &operator=(bitstring const &);

    ~bitstring();

    void append(bool symbol);
    void append(uint8_t word);
    void append(bitstring &);
    void detach();
    std::string data();
    inline size_t length() const
    {
        return _length;
    }

    bool operator[](size_t index) const
    {
        return (storage[index / 64] & (1ull << (index % 64ull))) != 0;
    }
    std::string split_string();

    char get_ordered_char(size_t index) const;

    void reserve(size_t new_size);

private:
    size_t _length;
    std::vector<uint64_t> storage;
};

#endif //HUFFMAN_BITSTRING_H
