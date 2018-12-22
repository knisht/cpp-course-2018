#include "include/document.h"
#include <QDebug>

#include <QSet>
Document::Document() {}

Document::Document(QString filename) : filename(filename), trigramOccurrences{}
{
}

// Document::Document(Document const &other)
//    : filename(other.filename), trigramOccurrences(other.trigramOccurrences)
//{
//}

// Document &Document::operator=(Document const &other)
//{
//    filename = other.filename;
//    trigramOccurrences = other.trigramOccurrences;
//    return *this;
//}
Document::Document(Document &&other)
    : filename(std::move(other.filename)),
      trigramOccurrences(std::move(other.trigramOccurrences))
{
}

size_t Document::DocumentHash::operator()(Document const &target) const
{
    return qHash(target.filename);
}

// Document &Document::operator=(Document &&other)
//{
//    Document temporary(other);
//    swap(*this, temporary);
//    return *this;
//}

void Document::add(Trigram const &trigram) const
{
    trigramOccurrences.push_back(trigram);
}

bool Document::contains(Trigram const &trigram) const
{
    auto result = std::lower_bound(trigramOccurrences.begin(),
                                   trigramOccurrences.end(), trigram);
    return result != trigramOccurrences.end() && *(result) == trigram;
}

void Document::sort() const
{
    std::sort(trigramOccurrences.begin(), trigramOccurrences.end());
}

bool nonTrivial(Document const &document)
{
    return document.trigramOccurrences.size() > 0;
}

void swap(Document &first, Document &second)
{
    swap(first.filename, second.filename);
    swap(first.trigramOccurrences, second.trigramOccurrences);
}

bool operator==(Document const &a, Document const &b)
{
    return a.filename == b.filename;
}
