#include "../../include/bitstring.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

bitstring::bitstring() : _length(0), storage()
{
}

bitstring::bitstring(bitstring const &source) : _length(source._length),
                                                storage(source.storage)
{
}

bitstring &bitstring::operator=(bitstring const &other)
{
    if (&other == this)
        return *this;
    bitstring tmp(other);
    std::swap(_length, tmp._length);
    swap(storage, tmp.storage);
    return *this;
}

bitstring::~bitstring() = default;

void bitstring::append(bool symbol)
{
    if (_length >= storage.size() * 64) {
        storage.push_back(0);
    }
    if (symbol)
        storage[_length / 64] += (1ull << (_length % 64ull));
    ++_length;
}

void bitstring::append(uint8_t word)
{

    if (_length + 8u >= storage.size() * 64) {
        storage.push_back(0);
    }
    size_t remainder = _length % 64ull;
    uint64_t remainder_completed = 64ull - remainder;
    if (remainder_completed < 8ull) {
        storage[(_length) / 64u] |= (word & ((1ull << remainder_completed) - 1ull)) << remainder;
        storage[(_length / 64u) + 1u] = (static_cast<uint64_t>(word) >> remainder_completed) & ((1ull << (8ull - remainder_completed)) - 1ull);
    } else {
        storage[_length / 64] += static_cast<uint64_t>(word) << remainder;
    }

    _length += 8;
}

void bitstring::append(bitstring &other)
{

    if (storage.size() * 64 <= _length + other._length) {
        storage.resize(std::max(storage.size() * 2, (_length + other._length + 63) / 64), 0);
    }
    size_t remainder = (64 - (_length % 64)) % 64;
    size_t previous_block = (_length / 64);
    for (size_t index = 0; index < (other._length + 63ull) / 64ull; ++index) {
        if (remainder) {
            storage[previous_block + index] |= (other.storage[index] & ((1ull << remainder) - 1ull)) << (64ull - remainder);
            if (!(index == (other._length / 64ull) && (other._length % 64ull) < remainder))
                storage[previous_block + index + 1] = (other.storage[index] >> remainder) & ((1ull << (64ull - remainder)) - 1ull);
        } else {
            storage[previous_block + index] = other.storage[index];
        }
    }
    _length = _length + other._length;
}

void bitstring::detach()
{
    --_length;
}

std::string bitstring::data()
{
    std::string result;
    for (uint64_t term : storage) {
        result.append((reinterpret_cast<char *>(&term)), 8);
    }
    return result;
}

std::string bitstring::split_string()
{
    size_t oldlength = _length;
    std::string tmp = data();
    while (tmp.size() * 8 > oldlength + 7)
        tmp.pop_back();
    *this = bitstring{};

    if (oldlength % 8 != 0) {
        append(static_cast<uint8_t>(tmp.back()));
        _length = oldlength % 8;
        tmp.pop_back();
    }
    return tmp;
}

char bitstring::get_ordered_char(size_t index) const
{
    return static_cast<char>(storage[index / 8] >> (8 * (index % 8)));
}

void bitstring::reserve(size_t new_size)
{
    storage.reserve(new_size);
}
