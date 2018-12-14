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

static const qint32 BUFF_SIZE = 1 << 20;

void TrigramIndex::printDocuments()
{
    std::cout << "Files at all: " << documents.size() << std::endl;
    for (auto &it : documents) {
        std::cout << it.filename.toStdString() << std::endl;
    }
}

TrigramIndex::TrigramIndex() {}

void TrigramIndex::unwrapTrigrams(Document &document)
{
    QFile fileInstance{QFileInfo(document.filename).absoluteFilePath()};
    std::ifstream ifs(document.filename.toStdString());
    if (!fileInstance.open(QFile::ReadOnly)) {
        qWarning() << "Could not open" << document.filename
                   << "| Reindex recommended";
        return;
    }
    qint32 fileSize = static_cast<qint32>(QFileInfo(document.filename).size());

    // NOTE: files with filesize <= 2 are ignored
    if (fileSize <= 2) {
        return;
    }
    size_t block_size = static_cast<size_t>(qMin(fileSize, BUFF_SIZE));
    std::string bytes;
    bytes.resize(static_cast<size_t>(block_size + 1), '\0');
    bytes.back() = '\1';
    char last[3];
    int passed = 0;

    while (fileSize > 0) {
        qint64 receivedBytes =
            fileInstance.read(&bytes[0], static_cast<qint64>(block_size));
        bool has_zero = (strlen(&bytes[0]) < block_size);
        for (size_t i = 0; i < static_cast<size_t>(receivedBytes) / 4; ++i) {
            if (bytes[4 * i] == 0) {
                has_zero = true;
            }
        }
        if (has_zero) {
            document.trigramOccurrences.clear();
            fileInstance.close();
            return;
        }
        if (passed > 0) {
            last[2] = bytes[0];
            document.trigramOccurrences.insert({last});
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            document.trigramOccurrences.insert({trigramBuf});
        }
#ifdef _OPENMP
//#pragma omp parallel for
// NOTE: strange things occur if above string is uncommented
#endif
        for (size_t i = 0; i < static_cast<size_t>(receivedBytes) - 2; ++i) {
            size_t trigram_code =
                static_cast<size_t>(
                    reinterpret_cast<unsigned char const &>(bytes[i]) << 16) +
                static_cast<size_t>(
                    reinterpret_cast<unsigned char const &>(bytes[i + 1])
                    << 8) +
                static_cast<size_t>(
                    reinterpret_cast<unsigned char const &>(bytes[i + 2]));
            document.trigramOccurrences.insert(trigram_code);
        }
        if (document.trigramOccurrences.size() > 200000) {
            document.trigramOccurrences.clear();
            fileInstance.close();
            return;
        }
        last[0] = bytes[block_size - 2];
        last[1] = bytes[block_size - 1];
        fileSize -= receivedBytes;
        block_size = static_cast<size_t>(
            qMin(static_cast<qint32>(block_size), fileSize));
        ++passed;
    }
    fileInstance.close();
}

void TrigramIndex::reprocessFile(QString const &filename)
{
    for (size_t i = 0; i < documents.size(); ++i) {
        if (documents[i].filename == filename) {
            documents[i].trigramOccurrences.clear();
            unwrapTrigrams(documents[i]);
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
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (!dirIterator.fileInfo().isDir()) {
            documents.push_back(
                Document(QFile(dirIterator.filePath()).fileName()));
        }
    }
    std::vector<size_t> realDocuments;
    for (size_t i = 0; i < documents.size(); ++i) {
        realDocuments.push_back(i);
        for (size_t j = 0; j < this->documents.size(); ++j) {
            if (documents[i].filename == this->documents[j].filename) {
                realDocuments.pop_back();
            }
        }
    }
    qDebug() << realDocuments.size();
    std::vector<QString> changedFiles;
    for (size_t docId : realDocuments) {
        this->documents.push_back(documents[docId]);
        unwrapTrigrams(this->documents[this->documents.size() - 1]);
        changedFiles.push_back(documents[docId].filename);
    }
    return changedFiles;
}

void TrigramIndex::flush() { documents.clear(); }

const std::vector<Document> &TrigramIndex::getDocuments() const
{
    return documents;
}
