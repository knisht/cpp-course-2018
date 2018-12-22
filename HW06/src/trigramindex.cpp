#include "include/trigramindex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_set>
#include <utility>

void TrigramIndex::printDocuments()
{
    std::cout << "Files at all: " << documents.size() << std::endl;
    for (auto &it : documents) {
        std::cout << it.filename.toStdString() << std::endl;
    }
}

TrigramIndex::TrigramIndex() {}

void TrigramIndex::flush() { documents.clear(); }

const std::vector<Document> &TrigramIndex::getDocuments() const
{
    return documents;
}

bool TrigramIndex::has_zero(char *buf, size_t expected_buf_size)
{
    return strlen(buf) < expected_buf_size;
}

void TrigramIndex::collectUnicodeSymbols(std::string::const_iterator &begin,
                                         std::string::const_iterator const &end,
                                         size_t &collector)
{
    for (; begin < end; ++begin) {
        if (is_unicode_independent(*begin)) {
            ++collector;
        }
    }
}
