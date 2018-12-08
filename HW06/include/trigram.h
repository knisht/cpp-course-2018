#ifndef TRIGRAM_H
#define TRIGRAM_H
#include <vector>
#include <string>

struct Trigram {

    Trigram(char *c_str);

    Trigram(std::string str);

    Trigram(size_t code);

    friend bool operator<(Trigram const &a, Trigram const &b);

    friend bool operator>(Trigram const &a, Trigram const &b);

    friend bool operator==(Trigram const &a, Trigram const &b);

    size_t code() const;
    bool substr(std::string const &) const;
    std::string toString() const;

    friend class TrigramIndex;

    struct TrigramHash {
        size_t operator()(Trigram const &trigram) const
        {
            return trigram.code();
        }
    };
private:
    size_t encode(const char *) const;
    size_t trigram_code;
};


#endif // TRIGRAM_H
