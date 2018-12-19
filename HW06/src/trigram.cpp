#include "include/trigram.h"
#include <iostream>

Trigram::Trigram(char *c_str)
{
    trigram_code[0] = c_str[0];
    trigram_code[1] = c_str[1];
    trigram_code[2] = c_str[2];
}

Trigram::Trigram(std::string str)
{
    trigram_code[0] = str[0];
    trigram_code[1] = str[1];
    trigram_code[2] = str[2];
}
// TODO: flush prograss bar when start indexing
Trigram::Trigram(uint32_t code)
{
    trigram_code[0] = static_cast<char>(code & ((1 << 8) - 1));
    trigram_code[1] = static_cast<char>((code >> 8) & ((1 << 8) - 1));
    trigram_code[2] = static_cast<char>((code >> 16) & ((1 << 8) - 1));
}

Trigram::Trigram(Trigram const &other)
{
    trigram_code[0] = other.trigram_code[0];
    trigram_code[1] = other.trigram_code[1];
    trigram_code[2] = other.trigram_code[2];
}

Trigram &Trigram::operator=(Trigram const &other)
{
    trigram_code[0] = other.trigram_code[0];
    trigram_code[1] = other.trigram_code[1];
    trigram_code[2] = other.trigram_code[2];
    return *this;
}

Trigram::Trigram(Trigram &&other)
{
    trigram_code[0] = other.trigram_code[0];
    trigram_code[1] = other.trigram_code[1];
    trigram_code[2] = other.trigram_code[2];
}

Trigram &Trigram::operator=(Trigram &&other)
{
    trigram_code[0] = other.trigram_code[0];
    trigram_code[1] = other.trigram_code[1];
    trigram_code[2] = other.trigram_code[2];
    return *this;
}

bool operator<(Trigram const &a, Trigram const &b)
{
    return a.code() < b.code();
}

bool operator>(Trigram const &a, Trigram const &b)
{
    return !(a < b) && !(a == b);
}

bool operator==(Trigram const &a, Trigram const &b)
{
    return a.code() == b.code();
}

uint32_t Trigram::code() const
{
    return static_cast<uint32_t>(trigram_code[0]) +
           (static_cast<uint32_t>(trigram_code[1]) << 8) +
           (static_cast<uint32_t>(trigram_code[2]) << 16);
}

uint32_t Trigram::encode(const char *target) const
{
    return (static_cast<uint32_t>(
                reinterpret_cast<unsigned char const &>(target[0]) << 16) +
            static_cast<uint32_t>(
                reinterpret_cast<unsigned char const &>(target[1]) << 8) +
            static_cast<uint32_t>(
                reinterpret_cast<unsigned char const &>(target[2])));
}

bool Trigram::substr(std::string const &target) const
{
    if (target.size() >= 3) {
        return false;
    }
    if (target.size() == 2) {
        if (((target[0] == trigram_code[0]) &&
             (target[1] == trigram_code[1])) ||
            ((target[1] == trigram_code[1]) &&
             (target[2] == trigram_code[2]))) {
            return true;
        } else {
            return false;
        }
    } else {
        return ((target[0] == trigram_code[0]) ||
                (target[1] == trigram_code[1]) ||
                (target[2] == trigram_code[2]));
    }
}

std::string Trigram::toString() const
{
    return {trigram_code[0], trigram_code[1], trigram_code[2]};
}
