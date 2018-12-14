#include "include/trigram.h"
#include <iostream>

Trigram::Trigram(char *c_str) : trigram_code(encode(c_str)) {}

Trigram::Trigram(std::string str) : trigram_code(encode(str.data())) {}

Trigram::Trigram(size_t code) : trigram_code(code) {}

bool operator<(Trigram const &a, Trigram const &b)
{
    return a.trigram_code < b.trigram_code;
}

bool operator>(Trigram const &a, Trigram const &b)
{
    return !(a < b) && !(a == b);
}

bool operator==(Trigram const &a, Trigram const &b)
{
    return a.trigram_code == b.trigram_code;
}

size_t Trigram::code() const { return trigram_code; }

size_t Trigram::encode(const char *target) const
{
    return static_cast<size_t>(target[0] << 16) +
           (static_cast<size_t>(target[1]) << 8) +
           (static_cast<size_t>(target[2]));
}

bool Trigram::substr(std::string const &target) const
{
    if (target.size() >= 3) {
        return false;
    }
    if (target.size() == 2) {
        size_t target_code = static_cast<size_t>((target[0] << 8) + target[1]);
        if (target_code == (trigram_code & ((1 << 16) - 1)) ||
            target_code == ((trigram_code >> 8) & ((1 << 16) - 1))) {
            return true;
        } else {
            return false;
        }
    } else {
        return static_cast<size_t>(target[0]) ==
                   (trigram_code & ((1 << 8) - 1)) ||
               static_cast<size_t>(target[0]) ==
                   ((trigram_code >> 8) & ((1 << 8) - 1)) ||
               static_cast<size_t>(target[0]) ==
                   ((trigram_code >> 16) & ((1 << 8) - 1));
    }
}

std::string Trigram::toString() const
{
    return {static_cast<char>(trigram_code >> 16),
            static_cast<char>(trigram_code >> 8),
            static_cast<char>(trigram_code)};
}
