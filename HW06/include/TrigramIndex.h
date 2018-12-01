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

    struct Trigram {
        QChar storage[3];
        friend bool operator<(Trigram const &a, Trigram const &b)
        {
            return a.storage[0] == b.storage[0]
                       ? a.storage[1] == b.storage[1]
                             ? a.storage[2] < b.storage[2]
                             : a.storage[1] < b.storage[1]
                       : a.storage[0] < b.storage[0];
        }
    };

    struct TrigramLocation : Trigram {
        std::vector<size_t> documents;
    };

    struct TrigramOccurrence : Trigram {
        std::vector<size_t> occurrences;
    };

    struct Document {
        // TODO: make set with comparator
        QString filename;
        std::set<TrigramOccurrence> trigrams;
    };

private:
    std::vector<TrigramLocation> trigram;
    // TODO: put documents on disk
    std::vector<Document> documents;
};

#endif // TRIGRAM_INDEX_H
