#include "include/document.h"

Document::Document() {}

Document::Document(QString filename) : filename(filename), trigramOccurrences{}
{
}

void Document::add(Trigram const &trigram)
{
    trigramOccurrences.push_back(trigram);
}

bool Document::contains(Trigram const &trigram)
{
    return *(std::lower_bound(trigramOccurrences.begin(),
                              trigramOccurrences.end(), trigram)) == trigram;
}

void Document::sort()
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
