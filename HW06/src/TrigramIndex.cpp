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

void TrigramIndex::coutTime(decltype(std::chrono::steady_clock::now()) start,
                            std::string msg)
{
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << msg << ": " << duration.count() << std::endl;
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

const qint32 BUFFER_SIZE = 1 << 20;

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
    qint32 block_size = qMin(fileSize, BUFFER_SIZE) + 1;
    std::string bytes;
    bytes.resize(block_size, '\0');
    bytes.back() = '\1';
    char last[3];
    int passed = 0;

    auto start = std::chrono::steady_clock::now();
    while (fileSize > 0) {
        size_t receivedBytes = fileInstance.read(&bytes[0], block_size);
        bool has_zero = (strlen(&bytes[0]) < block_size);
        for (int i = 0; i < receivedBytes / 4; ++i) {
            if (bytes[4 * i] == 0) {
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
#ifdef PARALLEL_INDEX
#pragma omp parallel for
#endif
        for (size_t i = 0; i < receivedBytes - 2; ++i) {
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
        fileSize -= receivedBytes;
        block_size = qMin(block_size, fileSize);
        ++passed;
    }
    fileInstance.close();
}

void TrigramIndex::mergeUnorderedSets(QSet<size_t> &destination,
                                      QSet<size_t> const &source)
{
    std::vector<size_t> fileIds;
    for (size_t fileId : destination) {
        if (source.contains(fileId) == 0) {
            fileIds.push_back(fileId);
        }
    }
    for (size_t fileId : fileIds) {
        destination.remove(fileId);
    }
}

void TrigramIndex::reprocessFile(QString const &filename)
{
    for (size_t i = 0; i < documents.size(); ++i) {
        if (documents[i].filename == filename) {
            qDebug() << "I'M REPROCESSING!!" << documents[i].filename;
            for (Trigram const &trigram : documents[i].trigramOccurrences) {
                trigramsInFiles[trigram].remove(i);
            }
            unwrapTrigrams(documents[i]);
            for (Trigram const &trigram : documents[i].trigramOccurrences) {
                trigramsInFiles[trigram].insert(i);
            }
            return;
        }
    }
    // so new file added
    documents.push_back(Document{filename});
    unwrapTrigrams(documents.back());
    for (Trigram const &trigram : documents.back().trigramOccurrences) {
        trigramsInFiles[trigram].insert(documents.size() - 1);
    }
}

void TrigramIndex::flush()
{
    documents.clear();
    trigramsInFiles.clear();
}

const std::vector<TrigramIndex::Document> &TrigramIndex::getDocuments() const
{
    return documents;
}
