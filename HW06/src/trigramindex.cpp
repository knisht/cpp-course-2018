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

void TrigramIndex::reprocessFile(QString const &filename)
{
    for (size_t i = 0; i < documents.size(); ++i) {
        if (documents[i].filename == filename) {
            documents[i].trigramOccurrences.clear();
            //            unwrapTrigrams(documents[i]);
            return;
        }
    }
}

std::vector<QString> TrigramIndex::reprocessDirectory(QString const &filename)
{
    QDirIterator dirIterator(filename, QDir::NoFilter | QDir::Hidden |
                                           QDir::NoDotAndDotDot |
                                           QDir::NoDotDot);
    std::vector<Document> documents;
    std::vector<QString> changedFiles;
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (!dirIterator.fileInfo().isDir()) {
            documents.push_back(
                Document(QFile(dirIterator.filePath()).fileName()));
        } else {
            changedFiles.push_back(
                QFileInfo(dirIterator.filePath()).absoluteFilePath());
        }
    }
    std::vector<size_t> realDocuments;
    for (size_t i = 0; i < documents.size(); ++i) {
        realDocuments.push_back(i);
        for (size_t j = 0; j < this->documents.size(); ++j) {
            if (QFileInfo(documents[i].filename).absoluteFilePath() ==
                this->documents[j].filename) {
                realDocuments.pop_back();
            }
        }
    }
    for (size_t docId : realDocuments) {
        //        unwrapTrigrams(documents[docId]);
        if (documents[docId].trigramOccurrences.size() > 0) {
            changedFiles.push_back(documents[docId].filename);
            this->documents.push_back(std::move(documents[docId]));
        }
    }
    return changedFiles;
}

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
