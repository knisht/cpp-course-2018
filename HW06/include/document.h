#ifndef LIBRARIAN_DOCUMENT_H
#define LIBRARIAN_DOCUMENT_H

#include "trigram.h"
#include <QString>
#include <unordered_set>

struct Document {
    QString filename;
    std::vector<Trigram> trigramOccurrences;
    explicit Document(QString filename);
    Document();

    void sort();

    bool contains(Trigram const &trigram);

    friend bool nonTrivial(Document const &document);
    friend void swap(Document &first, Document &second);

    void add(Trigram const &trigram);
};
bool nonTrivial(Document const &document);
void swap(Document &first, Document &second);
#endif
