#include "../include/TrigramIndex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <vector>

const qint32 BUF_SIZE = 1 << 20;

std::vector<TrigramIndex::Document> getFileEntries(QString const &root);

void unwrapTrigrams(TrigramIndex::Document &document);

TrigramIndex::TrigramIndex(QString const &root)
{
    // TODO: make branch cut by number of trigrams
    documents = getFileEntries(root);
    for (size_t i = 0; i < documents.size(); ++i) {
        unwrapTrigrams(documents[i]);
        for (auto &trigram : documents[i].trigramOccurrences) {
            if (trigramsInFiles.count(trigram.first) == 0) {
                trigramsInFiles[trigram.first] = std::vector<size_t>{};
            }
            trigramsInFiles[trigram.first].push_back(i);
        }
    }
}

std::vector<TrigramIndex::Document> getFileEntries(QString const &root)
{
    QDirIterator dirIterator(root, QDir::NoFilter,
                             QDirIterator::Subdirectories);
    std::vector<TrigramIndex::Document> documents;
    while (dirIterator.hasNext()) {
        documents.push_back(
            TrigramIndex::Document(QFile(dirIterator.next()).fileName()));
    }
    return documents;
}

void unwrapTrigrams(TrigramIndex::Document &document)
{
    QFile fileInstance{document.filename};
    // TODO: make error notifying
    fileInstance.open(QFile::ReadOnly);
    qint32 fileSize = static_cast<qint32>(fileInstance.size());
    qint32 block_size = qMin(fileSize, BUF_SIZE);
    QByteArray bytes(block_size, '\0');
    char last[3];
    // TODO: make small file processing
    int passed = 0;
    while (fileSize > 0) {
        fileInstance.read(bytes.data(), block_size);
        if (passed > 0) {
            last[2] = bytes[0];
            TrigramIndex::Trigram trigram{last[0], last[1], last[2]};
            if (document.trigramOccurrences.count(trigram) == 0) {
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size - 2));
            trigram = {last[1], last[2], bytes[1]};
            if (document.trigramOccurrences.count(trigram) == 0) {
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size - 1));
        }
        for (int i = 0; i < qMin(fileSize, block_size) - 2; ++i) {
            // FIXME: Copypast
            // FIXME: Slow
            TrigramIndex::Trigram trigram{bytes[i], bytes[i + 1], bytes[i + 2]};
            if (document.trigramOccurrences.count(trigram) == 0) {
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size + i));
        }
        fileSize -= block_size;
        ++passed;
    }
}

std::vector<TrigramIndex::SubstringOccurrence>
TrigramIndex::findSubstring(QString const &target) const
{
    // And so:
    // split target by trigrams
    // find all relevant documents
    // manually parse documents from occurrence places
    // kek after it
}
