#include "../../include/bitstring.h"
#include <algorithm>
#include <cstring>
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
    std::swap(storage, tmp.storage);
    return *this;
}

bitstring::~bitstring() = default;

void bitstring::append(bool symbol)
{
    if (_length >= storage.length() * 8) {
        storage.push_back('\0');
    }
    if (symbol)
        storage[_length / 8] += (1 << (_length % 8));
    ++_length;
}

void bitstring::append(uint8_t word)
{
    if (_length + 8u >= storage.length() * 8) {
        storage.push_back('\0');
    }
    uint8_t delta = _length % 8;
    uint8_t delta_complete = 8u - delta;
    if (delta) {
        storage[((_length) / 8)] |= (word & ((1u << delta_complete) - 1u)) << delta;
        storage[(_length / 8) + 1] = (word >> delta_complete) & ((1u << delta) - 1u);
    } else {
        storage[_length / 8] = reinterpret_cast<char &>(word);
    }
    _length += 8;
}

void bitstring::append(bitstring const &other)
{
    if (storage.length() * 8 <= _length + other._length) {
        storage.resize(std::max(storage.length() * 2, (_length + other._length + 7) / 8), '\0');
    }
    size_t delta = (((_length + 7) / 8) * 8) - _length;
    size_t lastindex = (_length / 8);
    for (size_t index = 0; index < (other._length + 7u) / 8u; ++index) {
        if (delta) {
            storage[lastindex + index] |= (other.storage[index] & ((1u << delta) - 1u)) << (8u - delta);
            if (!(index == (other._length / 8u) && (other._length % 8u) < delta))
                storage[lastindex + index + 1] = (other.storage[index] >> delta) & ((1u << (8u - delta)) - 1u);
        } else {
            storage[lastindex + index] = other.storage[index];
        }
    }
    _length = _length + other._length;
}

std::string &bitstring::data()
{
    return storage;
}

size_t bitstring::length() const
{
    return _length;
}

bool bitstring::operator[](size_t index) const
{
    uint8_t target = static_cast<uint8_t>(storage[index / 8]);
    return (target & (1u << (index % 8)));
}

char bitstring::get_char(size_t index) const
{
    return storage[index];
}

std::string bitstring::split_string()
{
    size_t oldlength = _length;
    std::string tmp = storage.substr(0, (_length + 7) / 8);
    *this = bitstring{};

    if (oldlength % 8 != 0) {
        append(reinterpret_cast<uint8_t &>(tmp.back()));
        _length = oldlength % 8;
        tmp.pop_back();
    }
    return tmp;
}

void bitstring::detach()
{
    --_length;
}
