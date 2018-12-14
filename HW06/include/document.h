#ifndef LIBRARIAN_DOCUMENT_H
#define LIBRARIAN_DOCUMENT_H

#include "trigram.h"
#include <QString>
#include <unordered_set>

struct Document {
    QString filename;
    std::unordered_set<Trigram, Trigram::TrigramHash> trigramOccurrences;
    explicit Document(QString filename);
};

#endif
