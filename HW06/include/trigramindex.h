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
    //    std::vector<SubstringOccurrence> findSubstring(QString const &);
    void flush();

    template <typename Owner>
    void setUp(QString const &root, TaskContext<Owner, qsizetype> &usualContext,
               TaskContext<Owner, QString const &> &dirContext)
    {
        // bare function for testing
        auto documents = getFileEntries(root, dirContext);
        calculateTrigrams(documents, usualContext);
        setUpDocuments(documents, usualContext);
    }

    template <typename T>
    std::vector<SubstringOccurrence>
    findSubstring(QString const &substring, TaskContext<T, qsizetype> &context)
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

#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context.isTaskCancelled()) {
                continue;
            }
            unwrapTrigrams(documents[i]);
            std::invoke(context.callOnSuccess, context.caller, 1);
        }
    }

    template <typename T>
    void setUpDocuments(std::vector<Document> &documents,
                        TaskContext<T, qsizetype> &context)
    {
        for (size_t i = 0; i < documents.size() && !context.isTaskCancelled();
             ++i) {
            if (documents[i].trigramOccurrences.size() > 0) {
                this->documents.push_back(std::move(documents[i]));
            }
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
                containsAll &=
                    (documents[i].trigramOccurrences.count(trigram) != 0);
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
    std::vector<SubstringOccurrence>
    findOccurrencesInFiles(std::vector<size_t> fileIds,
                           std::string const &target,
                           TaskContext<T, qsizetype> &context)
    {
        std::boyer_moore_searcher processed_target(target.begin(),
                                                   target.end());
        std::vector<SubstringOccurrence> result;
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (context.isTaskCancelled()) {
                return {};
            }
            std::vector<size_t> vec =
                findExactOccurrences(documents[fileIds[i]], processed_target,
                                     target.size(), context);
            if (vec.size() > 0) {
                result.push_back(
                    SubstringOccurrence(documents[fileIds[i]].filename));
                result.back().occurrences = std::move(vec);
            }
        }
        return result;
    }

    template <typename T, typename Q>
    static std::vector<size_t>
    findExactOccurrences(Document const &doc,
                         std::boyer_moore_searcher<Q> target,
                         size_t target_size, TaskContext<T, qsizetype> &context)
    {
        QFile fileInstance(doc.filename);
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
            occurrencePosition = std::search(
                buf.begin() + blockSize - target_size, buf.end(), target);
            while (occurrencePosition != buf.end() &&
                   !context.isTaskCancelled()) {
                for (; lastOccurrencePosition < occurrencePosition;
                     ++lastOccurrencePosition) {
                    if (is_unicode_independent(*lastOccurrencePosition)) {
                    }
                }
                result.push_back(numchars);
                occurrencePosition =
                    std::search(++occurrencePosition, buf.end(), target);
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

private:
    template <typename T>
    void nothing(T)
    {
    }
    static inline bool is_unicode_independent(char c) noexcept
    {
        return static_cast<unsigned int>(reinterpret_cast<unsigned char &>(c)) <
                   0x80 ||
               static_cast<unsigned int>(reinterpret_cast<unsigned char &>(c)) >
                   0xbf;
    }

    static void unwrapTrigrams(Document &document);

    static const qint32 BUF_SIZE = 1 << 20;
    std::vector<Document> documents;
};
#endif // TRIGRAM_INDEX_H
