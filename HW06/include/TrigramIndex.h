#ifndef TRIGRAM_INDEX_H
#define TRIGRAM_INDEX_H

#include <QString>
#include <map>
#include <set>
#include <vector>

class TrigramIndex
{
public:
    TrigramIndex(QString const &root);

    struct SubstringOccurrence {
        QString filename;
        std::vector<size_t> occurrences;
    };

    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;

    struct Trigram {
        char storage[3];
        friend bool operator<(Trigram const &a, Trigram const &b)
        {
            return a.storage[0] == b.storage[0]
                       ? a.storage[1] == b.storage[1]
                             ? a.storage[2] < b.storage[2]
                             : a.storage[1] < b.storage[1]
                       : a.storage[0] < b.storage[0];
        }
    };

    struct Document {
        // TODO: make set with comparator
        QString filename;
        std::map<Trigram, std::vector<size_t>> trigramOccurrences;
        Document(QString filename) : filename(filename), trigramOccurrences{} {}
    };

private:
    std::map<Trigram, std::vector<size_t>> trigramsInFiles;
    // TODO: put documents on disk
    std::vector<Document> documents;
};

#endif // TRIGRAM_INDEX_H
