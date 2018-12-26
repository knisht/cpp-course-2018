#ifndef LIBRARIAN_DOCUMENT_H
#define LIBRARIAN_DOCUMENT_H

#include "trigram.h"
#include <QString>
#include <unordered_set>

struct Document {
    static void sort(std::vector<Trigram> &vec)
    {
        std::sort(vec.begin(), vec.end());
    }

    static bool contains(std::vector<Trigram> const &trigramVector,
                         Trigram const &target)
    {
        auto result = std::lower_bound(trigramVector.begin(),
                                       trigramVector.end(), target);
        return result != trigramVector.end() && *(result) == target;
    }

    struct QStringHash {
        size_t operator()(QString const &target) const;
    };
};
#endif
