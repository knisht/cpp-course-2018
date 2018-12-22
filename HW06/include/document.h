#ifndef LIBRARIAN_DOCUMENT_H
#define LIBRARIAN_DOCUMENT_H

#include "trigram.h"
#include <QString>
#include <unordered_set>

struct Document {
    QString filename;
    mutable std::vector<Trigram> trigramOccurrences;

    explicit Document(QString filename);

    //    Document(Document const &other);
    //    Document &operator=(Document const &other);
    Document(Document &&other);
    Document();

    void sort() const;

    bool contains(Trigram const &trigram) const;

    friend bool nonTrivial(Document const &document);
    friend void swap(Document &first, Document &second);
    friend bool operator==(Document const &a, Document const &b);
    struct DocumentHash {
        size_t operator()(Document const &target) const;
    };

    void add(Trigram const &trigram) const;
};
bool nonTrivial(Document const &document);
void swap(Document &first, Document &second);
bool operator==(Document const &a, Document const &b);
#endif
