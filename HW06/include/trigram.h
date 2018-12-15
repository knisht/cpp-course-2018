#ifndef TRIGRAM_H
#define TRIGRAM_H
#include <string>
#include <vector>

struct Trigram {

    Trigram(char *c_str);

    Trigram(std::string str);

    Trigram(uint32_t code);

    friend bool operator<(Trigram const &a, Trigram const &b);

    friend bool operator>(Trigram const &a, Trigram const &b);

    friend bool operator==(Trigram const &a, Trigram const &b);

    uint32_t code() const;
    bool substr(std::string const &) const;
    std::string toString() const;

    friend class TrigramIndex;
    friend struct TrigramHash;

    struct TrigramHash {
        inline size_t operator()(Trigram const &trigram) const
        {
            return static_cast<size_t>(trigram.code());
        }
    };

private:
    uint32_t encode(const char *) const;
    char trigram_code[3];
};

#endif // TRIGRAM_H
