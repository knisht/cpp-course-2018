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

        friend bool operator==(SubstringOccurrence const &a,
                               SubstringOccurrence const &b)
        {
            return a.filename == b.filename && a.occurrences == b.occurrences;
        }
    };

    TrigramIndex operator=(TrigramIndex const &) = delete;

    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;

    struct Trigram {

        Trigram(char *c_str) { memcpy(storage, c_str, 3); }

        Trigram(std::string str)
        {
            assert(str.size() >= 3);
            memcpy(storage, str.data(), 3);
        }

        friend bool operator<(Trigram const &a, Trigram const &b)
        {
            return a.storage[0] == b.storage[0]
                       ? a.storage[1] == b.storage[1]
                             ? a.storage[2] < b.storage[2]
                             : a.storage[1] < b.storage[1]
                       : a.storage[0] < b.storage[0];
        }

        const char *data() const { return storage; }

    private:
        char storage[3];
    };

    struct Document {
        QString filename;
        std::map<Trigram, std::vector<size_t>> trigramOccurrences;
        Document(QString filename) : filename(filename), trigramOccurrences{} {}
    };

private:
    std::map<Trigram, std::vector<size_t>> trigramsInFiles;
    // TODO: put documents on disk
    std::vector<Document> documents;

    std::vector<TrigramIndex::SubstringOccurrence>
    smallStringProcess(std::string target) const;
};
#endif // TRIGRAM_INDEX_H
