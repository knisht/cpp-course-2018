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

TrigramIndex::TrigramIndex() {}

void TrigramIndex::flush() { documents.clear(); }

TrigramIndex::IndexMap const &TrigramIndex::getDocuments() const
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
