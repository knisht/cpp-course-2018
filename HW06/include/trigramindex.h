#ifndef TRIGRAM_INDEX_H
#define TRIGRAM_INDEX_H

#include "trigram.h"
#include <QDebug>
#include <QDirIterator>
#include <QEventLoop>
#include <QString>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SubstringOccurrence;

Q_DECLARE_METATYPE(SubstringOccurrence)

class SubstringOccurrence : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE SubstringOccurrence()
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence(QString const &string,
                                    std::vector<size_t> vec)
        : filename(string), occurrences(vec)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence(SubstringOccurrence const &other)
        : filename(other.filename), occurrences(other.occurrences)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence operator=(SubstringOccurrence const &other)
    {
        qRegisterMetaType<SubstringOccurrence>();
        filename = other.filename;
        occurrences = other.occurrences;
        return *this;
    }

    QString filename;
    std::vector<size_t> occurrences;

    friend bool operator==(SubstringOccurrence const &a,
                           SubstringOccurrence const &b)
    {
        return a.filename == b.filename && a.occurrences == b.occurrences;
    }
};

template <class Caller, typename... Arg>
struct TaskContext {
    // Parent must support getTransactionalId
    // TODO: private, caller initialization?
    size_t transactionalId;
    Caller *caller;
    void (Caller::*callOnSuccess)(Arg...);

    bool isTaskCancelled()
    {
        return caller->getTransactionalId() != transactionalId;
    }
};

class TrigramIndex
{
public:
    TrigramIndex();
    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;
    void printDocuments();
    void setUp(QString const &path);

    size_t getTransactionalId();

    struct Document {
        QString filename;
        std::unordered_set<Trigram, Trigram::TrigramHash> trigramOccurrences;
        explicit Document(QString filename)
            : filename(filename), trigramOccurrences{}
        {
        }
    };

    template <typename T>
    static std::vector<Document>
    getFileEntries(QString const &root, TaskContext<T, qsizetype> *context,
                   TaskContext<T, QString const &> *directoryHandler)
    {
        QDirIterator dirIterator(root,
                                 QDir::NoFilter | QDir::Hidden |
                                     QDir::NoDotAndDotDot | QDir::NoDotDot,
                                 QDirIterator::Subdirectories);
        std::vector<TrigramIndex::Document> documents;
        while (dirIterator.hasNext() && !context->isTaskCancelled()) {
            dirIterator.next();
            if (dirIterator.fileInfo().isDir()) {
                std::invoke(directoryHandler->callOnSuccess,
                            directoryHandler->caller, dirIterator.filePath());
            } else {
                documents.push_back(TrigramIndex::Document(
                    QFileInfo(dirIterator.filePath()).absoluteFilePath()));
            }
        }
        return documents;
    }

    template <typename T>
    static void calculateTrigrams(std::vector<Document> &documents,
                                  TaskContext<T, qsizetype> *context)
    {

#ifdef PARALLEL_INDEX
#pragma omp parallel for
#endif
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context->isTaskCancelled()) {
                continue;
            }
            unwrapTrigrams(documents[i]);
            std::invoke(context->callOnSuccess, context->caller, 1);
        }
    }

    static void coutTime(decltype(std::chrono::steady_clock::now()) start,
                         std::string msg = "");

    template <typename T>
    void setUpDocuments(std::vector<Document> &documents,
                        TaskContext<T, qsizetype> *context)
    {
        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < documents.size(); ++i) {
            if (documents[i].trigramOccurrences.size() > 0) {
                this->documents.push_back(std::move(documents[i]));
            } else {
                std::invoke(context->callOnSuccess, context->caller, 1);
            }
        }
        start = std::chrono::steady_clock::now();
        QSet<size_t> set;
        for (size_t i = 0; i < this->documents.size(); ++i) {
            if (context->isTaskCancelled()) {
                return;
            }
        }
    }

    template <typename T>
    std::vector<size_t> getCandidateFileIds(std::string const &target,
                                            TaskContext<T, qsizetype> *context)
    {
        std::unordered_set<Trigram, Trigram::TrigramHash> targetTrigrams;
        if (target.size() < 3) {
            std::unordered_set<size_t> files;
            //            for (auto &pair : trigramsInFiles) {
            //                if (context->isTaskCancelled()) {
            //                    return {};
            //                }
            //                if (pair.first.substr(target)) {
            //                    files.insert(pair.second.begin(),
            //                    pair.second.end());
            //                }
            //            }
            std::vector<size_t> result;
            result.reserve(result.size() + files.size());
            for (size_t i = 0; i < documents.size(); ++i) {
                if (context->isTaskCancelled()) {
                    return {};
                } else {
                    result.push_back(i);
                }
            }
            return result;
        }

        for (size_t i = 0;
             i < target.size() - 2 && !(context->isTaskCancelled()); ++i) {
            targetTrigrams.insert({&target.c_str()[i]});
        }
        //        QSet<size_t> neccesaryFiles;

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

        //        for (size_t)
        //            for (Trigram const &trigram : targetTrigrams) {
        //                if (context->isTaskCancelled()) {
        //                    return {};
        //                }
        //                if (trigramsInFiles.count(trigram) > 0) {
        //                    mergeUnorderedSets(neccesaryFiles,
        //                                       trigramsInFiles.at(trigram));
        //                }
        //            }
        //        for (auto &&it : neccesaryFiles) {
        //            result.push_back(it);
        //        }
        return result;
    }

    template <typename T, typename R>
    std::vector<SubstringOccurrence>
    findOccurrencesInFiles(std::vector<size_t> fileIds,
                           std::string const &target,
                           TaskContext<T, R> *context)
    {
        std::boyer_moore_searcher processed_target(target.begin(),
                                                   target.end());
        std::vector<SubstringOccurrence> result;
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (context->isTaskCancelled()) {
                return {};
            }
            auto vec =
                findExactOccurrences(documents[fileIds[i]], processed_target,
                                     target.size(), context);
            if (vec.size() > 0) {
                result.push_back({documents[fileIds[i]].filename, vec});
                //                std::invoke(context->callOnSuccess,
                //                context->caller,
                //                            occurrence);
            }
        }
        return result;
    }

    template <typename T, typename R, typename Q>
    static std::vector<size_t>
    findExactOccurrences(Document const &doc,
                         std::boyer_moore_searcher<Q> target,
                         size_t target_size, TaskContext<T, R> *context)
    {
        QFile fileInstance(doc.filename);
        fileInstance.open(QFile::ReadOnly);
        size_t fileSize = static_cast<size_t>(fileInstance.size());
        size_t blockSize = std::min(fileSize, static_cast<size_t>(BUF_SIZE));

        std::vector<size_t> result;
        auto time = std::chrono::steady_clock::now();
        std::string buf(blockSize * 2, '\0');
        fileInstance.read(&buf[0], blockSize);
        fileSize -= blockSize;
        std::string::iterator lastOccurrencePosition = buf.begin();
        std::string::iterator occurrencePosition =
            std::search(buf.begin(), buf.begin() + blockSize, target);
        size_t numchars = 0;
        while (occurrencePosition != buf.begin() + blockSize) {
            if (context->isTaskCancelled()) {
                return {};
            }
            for (; lastOccurrencePosition < occurrencePosition;
                 ++lastOccurrencePosition) {
                if (static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                        *lastOccurrencePosition)) < 0x80 ||
                    static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                        *lastOccurrencePosition)) > 0xbf) {
                    ++numchars;
                }
            }
            result.push_back(numchars);
            occurrencePosition = std::search(++occurrencePosition,
                                             buf.begin() + blockSize, target);
        }
        for (; lastOccurrencePosition < occurrencePosition - target_size;
             ++lastOccurrencePosition) {
            if (static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                    *lastOccurrencePosition)) < 0x80 ||
                static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                    *lastOccurrencePosition)) > 0xbf) {
                ++numchars;
            }
        }
        size_t passed = blockSize;

        while (fileSize > 0 && !context->isTaskCancelled()) {
            size_t receivedBytes =
                fileInstance.read(&buf[blockSize], blockSize);
            fileSize -= receivedBytes;

            lastOccurrencePosition = buf.begin() + blockSize - target_size;
            occurrencePosition = std::search(
                buf.begin() + blockSize - target_size, buf.end(), target);
            while (occurrencePosition != buf.end() &&
                   !context->isTaskCancelled()) {
                for (; lastOccurrencePosition < occurrencePosition;
                     ++lastOccurrencePosition) {
                    if (static_cast<unsigned int>(
                            reinterpret_cast<unsigned char &>(
                                *lastOccurrencePosition)) < 0x80 ||
                        static_cast<unsigned int>(
                            reinterpret_cast<unsigned char &>(
                                *lastOccurrencePosition)) > 0xbf) {
                        ++numchars;
                    }
                }
                result.push_back(numchars);
                occurrencePosition =
                    std::search(++occurrencePosition, buf.end(), target);
            }

            for (; lastOccurrencePosition < occurrencePosition - target_size;
                 ++lastOccurrencePosition) {
                if (static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                        *lastOccurrencePosition)) < 0x80 ||
                    static_cast<unsigned int>(reinterpret_cast<unsigned char &>(
                        *lastOccurrencePosition)) > 0xbf) {
                    ++numchars;
                }
            }
            passed += blockSize;
            memcpy(&buf[0], &buf[blockSize], blockSize);
        }
        fileInstance.close();
        return result;
    }

    static void mergeUnorderedSets(QSet<size_t> &destination,
                                   QSet<size_t> const &source);
    std::vector<SubstringOccurrence> findSubstring(QString const &);

    void reprocessFile(QString const &filename);
    std::vector<QString> reprocessDirectory(QString const &dirname);
    const std::vector<Document> &getDocuments() const;

    void flush();

private:
    bool valid;

    template <typename T>
    void nothing(T)
    {
    }

    static const qint32 BUF_SIZE = 1 << 20;
    void catchSubstring(SubstringOccurrence const &);
    // TODO: ifdef test
    std::vector<SubstringOccurrence> storage;
    std::vector<Document> documents;

    static void unwrapTrigrams(TrigramIndex::Document &document);
};
#endif // TRIGRAM_INDEX_H
