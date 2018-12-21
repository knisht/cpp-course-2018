#ifndef LIBRARIAN_TRIGRAM_INDEX_H
#define LIBRARIAN_TRIGRAM_INDEX_H

#include "document.h"
#include "substringoccurrence.h"
#include "taskcontext.h"
#include "trigram.h"
#include <QDebug>
#include <QDirIterator>
#include <QString>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

class TrigramIndex
{
public:
    TrigramIndex();
    TrigramIndex(TrigramIndex const &) = delete;
    TrigramIndex &operator=(TrigramIndex const &) = delete;

    void reprocessFile(QString const &filename);
    std::vector<QString> reprocessDirectory(QString const &dirname);
    const std::vector<Document> &getDocuments() const;
    void printDocuments();
    void flush();

    template <typename Owner>
    void setUp(QString const &root, TaskContext<Owner, qsizetype> &usualContext,
               TaskContext<Owner, QString const &> &dirContext)
    {
        // just function for testing
        auto documents = getFileEntries(root, dirContext);
        calculateTrigrams(documents, usualContext);
        //        setUpDocuments(documents, usualContext);
    }

    template <typename T>
    std::vector<QString> findSubstring(QString const &substring,
                                       TaskContext<T, qsizetype> &context)
    {
        return findOccurrencesInFiles(
            getCandidateFileIds(substring.toStdString(), context),
            substring.toStdString(), context);
    }

    template <typename T>
    static std::vector<Document>
    getFileEntries(QString const &root,
                   TaskContext<T, QString const &> &directoryHandler)
    {
        QDirIterator dirIterator(
            root, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        std::vector<Document> documents;
        while (dirIterator.hasNext() && !directoryHandler.isTaskCancelled()) {
            dirIterator.next();
            if (dirIterator.fileInfo().isDir()) {
                std::invoke(directoryHandler.callOnSuccess,
                            directoryHandler.caller, dirIterator.filePath());
            } else {
                documents.push_back(Document(
                    QFileInfo(dirIterator.filePath()).absoluteFilePath()));
            }
        }
        return documents;
    }

    template <typename T>
    static void calculateTrigrams(std::vector<Document> &documents,
                                  TaskContext<T, qsizetype> &context)
    {
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context.isTaskCancelled()) {
                continue;
            }
            //            unwrapTrigrams(documents[i]);
            documents[i].sort();
            std::invoke(context.callOnSuccess, context.caller, 1);
        }
    }

    template <typename T>
    std::vector<size_t> getCandidateFileIds(std::string const &target,
                                            TaskContext<T, qsizetype> &context)
    {
        if (target.size() < 3) {
            std::vector<size_t> result;
            for (size_t i = 0; i < documents.size(); ++i) {
                if (context.isTaskCancelled()) {
                    return {};
                } else {
                    result.push_back(i);
                }
            }
            return result;
        }

        std::unordered_set<Trigram, Trigram::TrigramHash> targetTrigrams;
        for (size_t i = 0;
             i < target.size() - 2 && !(context.isTaskCancelled()); ++i) {
            targetTrigrams.insert({&target[i]});
        }
        std::vector<size_t> result;
        for (size_t i = 0; i < documents.size(); ++i) {
            bool containsAll = true;
            for (Trigram const &trigram : targetTrigrams) {
                containsAll &= (documents[i].contains(trigram));
                if (!containsAll) {
                    break;
                }
            }
            if (containsAll) {
                result.push_back(i);
            }
        }
        return result;
    }

    struct Searcher {
        std::boyer_moore_searcher<std::string::const_iterator> core_searcher;
        size_t patternSize;
    };

    template <typename T>
    void
    findOccurrencesInFiles(std::vector<size_t> fileIds,
                           std::string const &target,
                           TaskContext<T, QString const &, size_t> &context)
    {
        Searcher searcher{
            std::boyer_moore_searcher<std::string::const_iterator>(
                target.begin(), target.end()),
            target.size()};
        std::vector<QString> result;
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (context.isTaskCancelled()) {
                return;
            }

            //            std::vector<size_t> vec = collectAllOccurrences(
            //                documents[fileIds[i]].filename, searcher,
            //                context);
            if (occurrenceExist(documents[fileIds[i]].filename, searcher,
                                context)) {
                std::invoke(context.callOnSuccess, context.caller,
                            documents[fileIds[i]].filename,
                            context.transactionalId);
            }
        }
    }

    template <typename T>
    std::vector<size_t>
    collectAllOccurrences(QString const &filename, Searcher const &searcher,
                          TaskContext<T, QString const &, size_t> &context)
    {
        std::vector<size_t> resultPositions;
        size_t passedUnicodeCharactersAmount = 0;
        auto collector = [&](std::string::const_iterator &begin,
                             std::string::const_iterator &end) -> bool {
            collectUnicodeSymbols(begin, end, passedUnicodeCharactersAmount);
            resultPositions.push_back(passedUnicodeCharactersAmount);
            return true;
        };

        auto finish = [&](std::string::const_iterator &begin,
                          std::string::const_iterator const &end) -> bool {
            collectUnicodeSymbols(begin, end, passedUnicodeCharactersAmount);
            return true;
        };
        processSubstringOccurrencesInFile(filename, searcher, collector, finish,
                                          context);
        return resultPositions;
    }

    template <typename T>
    bool occurrenceExist(QString const &filename, Searcher const &searcher,
                         TaskContext<T, QString const &, size_t> &context)
    {
        bool exist = false;
        auto checker = [&](std::string::const_iterator &begin [[gnu::unused]],
                           std::string::const_iterator &end
                           [[gnu::unused]]) -> bool {
            exist = true;
            return false;
        };
        auto finish = [&](std::string::const_iterator &begin [[gnu::unused]],
                          std::string::const_iterator const &end
                          [[gnu::unused]]) -> bool { return true; };
        processSubstringOccurrencesInFile(filename, searcher, checker, finish,
                                          context);
        return exist;
    }

    template <typename T>
    static void unwrapTrigrams(Document &document,
                               TaskContext<T, qsizetype> context)
    {
        static const qsizetype BUF_SIZE = 1 << 12;
        // Strange, above line is neccessary for debug build (otherwise it
        // doesn't compile)
        QFile fileInstance{QFileInfo(document.filename).absoluteFilePath()};
        if (!fileInstance.open(QFile::ReadOnly)) {
            qWarning() << "Could not open" << document.filename
                       << "| Reindex recommended";
            return;
        }
        qsizetype fileSize = QFileInfo(document.filename).size();

        // NOTE: files with filesize <= 2 are ignored
        if (fileSize <= 2) {
            std::invoke(context.callOnSuccess, context.caller, 1);
            return;
        }
        qsizetype blockSize = qMin(fileSize, BUF_SIZE);
        std::string bytes;
        bytes.resize(static_cast<size_t>(blockSize + 1), '\0');
        char last[3];
        int processedBytesAmount = 0;

        std::unordered_set<Trigram, Trigram::TrigramHash> foundTrigrams;

        while (fileSize > 0) {
            qint64 receivedBytes =
                fileInstance.read(&bytes[0], static_cast<qint64>(blockSize));
            if (has_zero(&bytes[0], static_cast<size_t>(receivedBytes))) {
                foundTrigrams.clear();
                break;
            }
            if (processedBytesAmount > 0) {
                // if it is not first iteration
                last[2] = bytes[0];
                foundTrigrams.insert({last});
                char trigramBuf[3] = {last[1], last[2], bytes[1]};
                foundTrigrams.insert({trigramBuf});
            }
            if (receivedBytes > 2) {
                for (size_t i = 0; i < static_cast<size_t>(receivedBytes) - 2;
                     ++i) {
                    foundTrigrams.insert(&bytes[i]);
                }
                if (foundTrigrams.size() > 200000) {
                    qDebug()
                        << document.filename << "is too big for processing";
                    foundTrigrams.clear();
                    break;
                }
                last[0] = bytes[static_cast<size_t>(blockSize) - 2];
                last[1] = bytes[static_cast<size_t>(blockSize) - 1];
            }
            fileSize -= receivedBytes;
            blockSize = qMin(blockSize, fileSize);
            ++processedBytesAmount;
        }
        for (Trigram const &t : foundTrigrams) {
            document.add(t);
        }
        document.sort();
        std::invoke(context.callOnSuccess, context.caller, 1);
    }

    template <typename T>
    void getFilteredDocuments(std::vector<Document> &candidateDocuments,
                              TaskContext<T, qsizetype> &context)
    {
        for (size_t i = 0;
             i < candidateDocuments.size() && !context.isTaskCancelled(); ++i) {
            if (nonTrivial(candidateDocuments[i])) {
                this->documents.push_back({});
                swap(this->documents.back(), candidateDocuments[i]);
                std::invoke(context.callOnSuccess, context.caller, 1);
            }
        }
    }

    friend class IndexDriver;

private:
    template <typename T>
    void nothing(T)
    {
    }

    template <typename T, typename ProgressFunction, typename FinishFunction>
    static void processSubstringOccurrencesInFile(
        QString const &filename, Searcher const &patternSearcher,
        ProgressFunction progress, FinishFunction finish,
        TaskContext<T, QString const &, size_t> &context)
    {
        QFile fileInstance(filename);
        fileInstance.open(QFile::ReadOnly);
        size_t fileSize = static_cast<size_t>(fileInstance.size());
        size_t blockSize = std::min(fileSize, static_cast<size_t>(BUF_SIZE));

        std::string buffer(blockSize * 2, '\0');
        fileInstance.read(&buffer[0], static_cast<qint64>(blockSize));
        fileSize -= blockSize;

        if (!processBuffer(buffer.cbegin(),
                           buffer.cbegin() + static_cast<qint64>(blockSize),
                           patternSearcher, progress, finish, context)) {
            return;
        }
        size_t passed = blockSize;
        while (fileSize > 0 && !context.isTaskCancelled()) {
            size_t receivedBytes = static_cast<size_t>(fileInstance.read(
                &buffer[blockSize], static_cast<qint64>(blockSize)));
            fileSize -= receivedBytes;
            if (!processBuffer(buffer.cbegin() +
                                   static_cast<qint64>(
                                       blockSize - patternSearcher.patternSize),
                               buffer.cbegin() + static_cast<ptrdiff_t>(
                                                     blockSize + receivedBytes),
                               patternSearcher, progress, finish, context)) {
                return;
            }
            passed += blockSize;
            memcpy(&buffer[0], &buffer[blockSize], blockSize);
        }
    }

    template <typename T, typename ProgressFunction, typename FinishFunction>
    static bool processBuffer(std::string::const_iterator begin,
                              std::string::const_iterator end,
                              Searcher const &searcher,
                              ProgressFunction process, FinishFunction finish,
                              TaskContext<T, QString const &, size_t> context)
    {
        std::string::const_iterator lastOccurrencePosition = begin;
        std::string::const_iterator currentOccurrencePosition =
            std::search(begin, end, searcher.core_searcher);
        while (currentOccurrencePosition != end) {
            if (context.isTaskCancelled()) {
                return false;
            }
            if (!process(lastOccurrencePosition, currentOccurrencePosition)) {
                return false;
            }
            currentOccurrencePosition = std::search(
                ++currentOccurrencePosition, end, searcher.core_searcher);
        }
        return finish(lastOccurrencePosition,
                      currentOccurrencePosition - searcher.patternSize);
    }

    static bool has_zero(char *buf, size_t expected_buf_size);

    static void collectUnicodeSymbols(std::string::const_iterator &begin,
                                      std::string::const_iterator const &end,
                                      size_t &collector);

    static inline bool is_unicode_independent(char c) noexcept
    {
        return (static_cast<unsigned int>(
                    reinterpret_cast<unsigned char &>(c)) < 0x80 ||
                static_cast<unsigned int>(
                    reinterpret_cast<unsigned char &>(c)) > 0xbf)
#ifdef __unix__
               && static_cast<unsigned int>(
                      reinterpret_cast<unsigned char &>(c)) != 0x0d
#endif
            ;
    }
    static const qsizetype BUF_SIZE = 1 << 12;
    std::vector<Document> documents;
};
#endif // TRIGRAM_INDEX_H
