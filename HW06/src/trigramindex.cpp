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

size_t TrigramIndex::getTransactionalId() { return 0; }

void TrigramIndex::setUp(QString const &root)
{
    // bare function for time measure, for example
    // TODO: remove trigramInFiles and therefore increase speed
    TaskContext<TrigramIndex, qsizetype> context{
        0, this, &TrigramIndex::nothing<qsizetype>};
    TaskContext<TrigramIndex, QString const &> dirContext{
        0, this, &TrigramIndex::nothing<QString const &>};
    auto documents = getFileEntries(root, &context, &dirContext);
    calculateTrigrams(documents, &context);
    setUpDocuments(documents, &context);
}

std::vector<SubstringOccurrence>
TrigramIndex::findSubstring(QString const &target)
{
    storage.clear();
    TaskContext<TrigramIndex, const SubstringOccurrence &> context{
        false, this, &TrigramIndex::catchSubstring};
    TaskContext<TrigramIndex, qsizetype> usualContext{false, this,
                                                      &TrigramIndex::nothing};
    auto files = getCandidateFileIds(target.toStdString(), &usualContext);
    findOccurrencesInFiles(files, target.toStdString(), &context);
    return storage;
}

const qint32 BUFFER_SIZE = 1 << 20;

TrigramIndex::TrigramIndex() : valid(false) {}

void TrigramIndex::unwrapTrigrams(TrigramIndex::Document &document)
{
    QFile fileInstance{QFileInfo(document.filename).absoluteFilePath()};
    std::ifstream ifs(document.filename.toStdString());
    // TODO: make error notifying
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
    qint32 block_size = qMin(fileSize, BUFFER_SIZE);
    std::string bytes;
    bytes.resize(block_size + 1, '\0');
    bytes.back() = '\1';
    char last[3];
    int passed = 0;

    //    auto start = std::chrono::steady_clock::now();
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
            fileInstance.close();
            return;
        }
        if (passed > 0) {
            last[2] = bytes[0];
            document.trigramOccurrences.insert({last});
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            document.trigramOccurrences.insert({trigramBuf});
        }
#ifdef PARALLEL_INDEX
//#pragma omp parallel for
// NOTE: strange things occurs if above string is uncommented
#endif
        for (size_t i = 0; i < receivedBytes - 2; ++i) {
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
        block_size = qMin(block_size, fileSize);
        ++passed;
    }
    fileInstance.close();
}

void TrigramIndex::mergeUnorderedSets(QSet<size_t> &destination,
                                      QSet<size_t> const &source)
{
    std::vector<size_t> fileIds;
    destination.intersect(source);
}

void TrigramIndex::reprocessFile(QString const &filename)
{
    qDebug() << "catch up" << filename;
    for (size_t i = 0; i < documents.size(); ++i) {
        if (documents[i].filename == filename) {
            qDebug() << "I'M REPROCESSING!!" << documents[i].filename << "with"
                     << documents[i].trigramOccurrences.size();
            for (Trigram const &trigram : documents[i].trigramOccurrences) {
                trigramsInFiles[trigram].remove(i);
                qDebug() << "in loop23";
            }
            documents[i].trigramOccurrences.clear();
            unwrapTrigrams(documents[i]);
            qDebug() << "now i'll do" << documents[i].trigramOccurrences.size();
            for (Trigram const &trigram : documents[i].trigramOccurrences) {
                trigramsInFiles[trigram].insert(i);
                qDebug() << "in loop24"
                         << QString::fromStdString(trigram.toString());
            }
            return;
        }
    }
}

std::vector<QString> TrigramIndex::reprocessDirectory(QString const &filename)
{
    QDirIterator dirIterator(filename, QDir::NoFilter | QDir::Hidden |
                                           QDir::NoDotAndDotDot |
                                           QDir::NoDotDot);
    // TODO: unordered map
    std::vector<TrigramIndex::Document> documents;
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (!dirIterator.fileInfo().isDir()) {
            documents.push_back(TrigramIndex::Document(
                QFile(dirIterator.filePath()).fileName()));
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
        qDebug() << "in loop2";
        for (const Trigram &trigram :
             this->documents.back().trigramOccurrences) {
            qDebug() << "in loop1";
            this->trigramsInFiles[trigram].insert(this->documents.size() - 1);
        }
        changedFiles.push_back(documents[docId].filename);
    }
    return changedFiles;
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
