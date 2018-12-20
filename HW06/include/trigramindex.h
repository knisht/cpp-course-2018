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
            targetTrigrams.insert({&target.c_str()[i]});
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

    template <typename T>
    void
    findOccurrencesInFiles(std::vector<size_t> fileIds,
                           std::string const &target,
                           TaskContext<T, QString const &, size_t> &context)
    {
        std::boyer_moore_searcher processed_target(target.begin(),
                                                   target.end());
        std::vector<QString> result;
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (context.isTaskCancelled()) {
                return;
            }
            std::vector<size_t> vec =
                findExactOccurrences(documents[fileIds[i]].filename,
                                     processed_target, target.size(), context);
            if (vec.size() > 0) {
                std::invoke(context.callOnSuccess, context.caller,
                            documents[fileIds[i]].filename,
                            context.transactionalId);
            }
        }
    }

    template <typename T, typename Q>
    static std::vector<size_t> findExactOccurrences(
        QString const &doc, std::boyer_moore_searcher<Q> target,
        size_t target_size, TaskContext<T, QString const &, size_t> &context)
    {
        QFile fileInstance(doc);
        fileInstance.open(QFile::ReadOnly);
        size_t fileSize = static_cast<size_t>(fileInstance.size());
        size_t blockSize = std::min(fileSize, static_cast<size_t>(BUF_SIZE));

        std::string buf(blockSize * 2, '\0');
        fileInstance.read(&buf[0], static_cast<qint64>(blockSize));
        fileSize -= blockSize;
        std::string::iterator lastOccurrencePosition = buf.begin();
        std::string::iterator occurrencePosition =
            std::search(buf.begin(), buf.begin() + blockSize, target);
        size_t numchars = 0;
        std::vector<size_t> result;
        // TODO: special processing for words with width 3
        while (occurrencePosition !=
               buf.begin() + static_cast<qint64>(blockSize)) {
            if (context.isTaskCancelled()) {
                return {};
            }
            for (; lastOccurrencePosition < occurrencePosition;
                 ++lastOccurrencePosition) {
                if (is_unicode_independent(*lastOccurrencePosition)) {
                    ++numchars;
                }
            }
            // TODO: fix bug w/ large file rendering
            result.push_back(numchars);
            occurrencePosition = std::search(++occurrencePosition,
                                             buf.begin() + blockSize, target);
        }
        for (; lastOccurrencePosition <
               occurrencePosition - static_cast<qint64>(target_size);
             ++lastOccurrencePosition) {
            if (is_unicode_independent(*lastOccurrencePosition)) {
                ++numchars;
            }
        }
        size_t passed = blockSize;

        while (fileSize > 0 && !context.isTaskCancelled()) {
            size_t receivedBytes = static_cast<size_t>(fileInstance.read(
                &buf[blockSize], static_cast<qint64>(blockSize)));
            fileSize -= receivedBytes;
            lastOccurrencePosition =
                buf.begin() + static_cast<qint64>(blockSize - target_size);
            auto limit =
                buf.begin() + static_cast<ptrdiff_t>(blockSize + receivedBytes);
            occurrencePosition = std::search(
                buf.begin() + blockSize - target_size, limit, target);
            while (occurrencePosition != limit && !context.isTaskCancelled()) {
                for (; lastOccurrencePosition < occurrencePosition;
                     ++lastOccurrencePosition) {
                    if (is_unicode_independent(*lastOccurrencePosition)) {
                        ++numchars;
                    }
                }
                result.push_back(numchars);
                occurrencePosition =
                    std::search(++occurrencePosition, limit, target);
            }

            for (; lastOccurrencePosition <
                   occurrencePosition - static_cast<qint64>(target_size);
                 ++lastOccurrencePosition) {
                if (is_unicode_independent(*lastOccurrencePosition)) {
                    ++numchars;
                }
            }
            passed += blockSize;
            memcpy(&buf[0], &buf[blockSize], blockSize);
        }
        fileInstance.close();
        return result;
    }

    template <typename T>
    static void unwrapTrigrams(Document &document,
                               TaskContext<T, qsizetype> context)
    {
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

        static const qint64 BUFF_SIZE = 1 << 12;
        qsizetype block_size = qMin(fileSize, BUFF_SIZE);
        std::string bytes;
        bytes.resize(static_cast<size_t>(block_size + 1), '\0');
        bytes.back() = '\1';
        char last[3];
        int passed = 0;

        std::unordered_set<Trigram, Trigram::TrigramHash> trigram_set;

        while (fileSize > 0) {
            qint64 receivedBytes =
                fileInstance.read(&bytes[0], static_cast<qint64>(block_size));
            bool has_zero =
                (strlen(&bytes[0]) < static_cast<size_t>(block_size));
            for (size_t i = 0; i < static_cast<size_t>(receivedBytes) / 4;
                 ++i) {
                if (bytes[4 * i] == 0) {
                    has_zero = true;
                }
            }
            if (has_zero) {
                document.trigramOccurrences.clear();
                break;
            }
            if (passed > 0) {
                last[2] = bytes[0];
                document.add({last});
                char trigramBuf[3] = {last[1], last[2], bytes[1]};
                document.add({trigramBuf});
            }
            if (receivedBytes > 2) {
                for (size_t i = 0; i < static_cast<size_t>(receivedBytes) - 2;
                     ++i) {
                    trigram_set.insert(&bytes[i]);
                }
                if (trigram_set.size() > 200000) {
                    qDebug() << "bad" << document.filename;
                    document.trigramOccurrences.clear();
                    return;
                }
                last[0] = bytes[static_cast<size_t>(block_size) - 2];
                last[1] = bytes[static_cast<size_t>(block_size) - 1];
            }
            fileSize -= receivedBytes;
            block_size = qMin(block_size, fileSize);
            ++passed;
        }
        for (Trigram t : trigram_set) {
            document.add(t);
        }
        document.sort();
        std::invoke(context.callOnSuccess, context.caller, 1);
    }
    //    static void unwrapTrigrams(Document &document);
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
