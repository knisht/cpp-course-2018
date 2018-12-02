#ifndef TRIGRAM_INDEX_H
#define TRIGRAM_INDEX_H

#include <QString>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

class TrigramIndex
{
public:
    TrigramIndex(QString const &root);

public:
    struct SubstringOccurrence {
        QString filename;
        std::vector<size_t> occurrences;

        friend bool operator==(SubstringOccurrence const &a,
                               SubstringOccurrence const &b)
        {
            return a.filename == b.filename && a.occurrences == b.occurrences;
        }
    };

    TrigramIndex operator=(TrigramIndex const &) = delete;

    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;

    struct Trigram {

        Trigram(char *c_str) : trigram_code(encode(c_str)) {}

        Trigram(std::string str) : trigram_code(encode(str.data())) {}

        Trigram(size_t code) : trigram_code(code) {}

        friend bool operator<(Trigram const &a, Trigram const &b)
        {
            return a.trigram_code < b.trigram_code;
        }

        friend bool operator==(Trigram const &a, Trigram const &b)
        {
            return a.trigram_code == b.trigram_code;
        }

        size_t code() const { return trigram_code; }
        bool substr(std::string const &) const;
        std::string toString() const;

    private:
        size_t encode(const char *) const;
        size_t trigram_code;
    };

private:
    struct TrigramHash {
        size_t operator()(Trigram const &trigram) const
        {
            return trigram.code();
        }
    };

public:
    struct Document {
        QString filename;
        std::unordered_map<Trigram, std::vector<size_t>, TrigramHash>
            trigramOccurrences;
        Document(QString filename) : filename(filename), trigramOccurrences{} {}
    };

private:
    std::unordered_map<Trigram, std::vector<size_t>, TrigramHash>
        trigramsInFiles;
    // TODO: put documents on disk
    std::vector<Document> documents;

    std::vector<TrigramIndex::SubstringOccurrence>
    smallStringProcess(std::string const &target) const;
};
#endif // TRIGRAM_INDEX_H
