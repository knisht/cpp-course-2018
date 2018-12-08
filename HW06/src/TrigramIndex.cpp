#include "include/TrigramIndex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <algorithm>
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

void TrigramIndex::nothing() {}

void TrigramIndex::catchSubstring(SubstringOccurrence const &substring)
{
    storage.push_back(substring);
}

void TrigramIndex::setUp(QString const &root)
{
    // TODO: enrich args in templates
    // bare function for time measure, for example
    TaskContext<TrigramIndex> context{false, this, &TrigramIndex::nothing};
    auto documents = getFileEntries(root, &context);
    calculateTrigrams(documents, &context);
    setUpDocuments(documents, &context);
}

std::vector<SubstringOccurrence>
TrigramIndex::findSubstring(QString const &target)
{
    storage.clear();
    TaskContext<TrigramIndex, const SubstringOccurrence &> context{
        false, this, &TrigramIndex::catchSubstring};
    TaskContext<TrigramIndex> usualContext{false, this, &TrigramIndex::nothing};
    auto files = getCandidateFileIds(target.toStdString(), &usualContext);
    findOccurrencesInFiles(files, target.toStdString(), &context);
    return storage;
}

const qint32 BUF_SIZE = 1 << 20;

TrigramIndex::TrigramIndex() : valid(false) {}

void TrigramIndex::unwrapTrigrams(TrigramIndex::Document &document)
{
    QFile fileInstance{document.filename};
    // TODO: make error notifying
    fileInstance.open(QFile::ReadOnly);
    qint32 fileSize = static_cast<qint32>(fileInstance.size());
    // NOTE: files with filesize <= 2 are ignored
    if (fileSize <= 2) {
        return;
    }
    qint32 block_size = qMin(fileSize, BUF_SIZE);
    std::string bytes;
    bytes.resize(block_size, '\0');
    char last[3];
    int passed = 0;
    while (fileSize > 0) {
        fileInstance.read(&bytes[0], block_size);
        bool has_zero = false;
        for (int i = 0; i < block_size; ++i) {
            if (bytes[i] == 0) {
                has_zero = true;
            }
        }
        if (has_zero) {
            document.trigramOccurrences.clear();
            return;
        }
        if (passed > 0) {
            last[2] = bytes[0];
            document.trigramOccurrences.insert({last});
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            document.trigramOccurrences.insert({trigramBuf});
        }
#pragma omp parallel for
        for (size_t i = 0; i < qMin(fileSize, block_size) - 2; ++i) {
            size_t trigram_code = static_cast<size_t>(bytes[i] << 16) +
                                  static_cast<size_t>(bytes[i + 1] << 8) +
                                  static_cast<size_t>(bytes[i + 2]);
            document.trigramOccurrences.insert(trigram_code);
        }
        if (document.trigramOccurrences.size() > 200000) {
            document.trigramOccurrences.clear();
            return;
        }
        last[0] = bytes[block_size - 2];
        last[1] = bytes[block_size - 1];
        fileSize -= block_size;
        block_size = qMin(block_size, fileSize);
        ++passed;
    }
    fileInstance.close();
}

void TrigramIndex::mergeVectorToList(std::list<size_t> &destination,
                                     std::vector<size_t> const &source)
{
    auto it = destination.begin();
    for (size_t currentIndex : source) {
        while (it != destination.end() && *it < currentIndex) {
            destination.erase(it++);
        }
        if (it == destination.end()) {
            return;
        }
        if (*it == currentIndex) {
            ++it;
        }
    }
}

std::vector<size_t>
TrigramIndex::findExactOccurrences(Document const &doc,
                                   std::string const &target)
{
    QFile fileInstance(doc.filename);
    fileInstance.open(QFile::ReadOnly);
    size_t fileSize = static_cast<size_t>(fileInstance.size());
    size_t blockSize = std::min(fileSize, static_cast<size_t>(BUF_SIZE));

    std::vector<size_t> result;
    // TODO: square, slow
    std::string buf(blockSize * 2, '\0');
    fileInstance.read(&buf[0], blockSize);
    fileSize -= blockSize;
    for (size_t i = 0; i < blockSize - target.size() + 1; ++i) {
        if (memcmp(&buf[i], &target[0], target.size()) == 0) {
            result.push_back(i);
        }
    }
    size_t passed = blockSize;
    while (fileSize > 0) {
        size_t receivedBytes = fileInstance.read(&buf[blockSize], blockSize);
        fileSize -= receivedBytes;
        for (size_t i = blockSize - target.size() + 1;
             i < blockSize + receivedBytes - target.size(); ++i) {
            if (memcmp(&buf[i], &target[0], target.size()) == 0) {
                result.push_back(i + passed - target.size());
            }
        }
        passed += blockSize;
        memcpy(&buf[0], &buf[blockSize], blockSize);
    }
    fileInstance.close();
    return result;
}
