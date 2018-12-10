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
    bool stopFlag;
    Caller *caller;
    void (Caller::*callOnSuccess)(Arg...);
};

class TrigramIndex
{
public:
    TrigramIndex();
    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;
    void printDocuments();
    void setUp(QString const &path);

    struct Document {
        QString filename;
        std::unordered_set<Trigram, Trigram::TrigramHash> trigramOccurrences;
        explicit Document(QString filename)
            : filename(filename), trigramOccurrences{}
        {
        }
    };

    template <typename T>
    static std::vector<Document> getFileEntries(QString const &root,
                                                TaskContext<T> *context)
    {
        QDirIterator dirIterator(root, QDir::NoFilter | QDir::Hidden,
                                 QDirIterator::Subdirectories);
        std::vector<TrigramIndex::Document> documents;
        while (dirIterator.hasNext() && !context->stopFlag) {
            dirIterator.next();
            if (dirIterator.fileInfo().isDir()) {
                continue;
            }
            documents.push_back(TrigramIndex::Document(
                QFile(dirIterator.filePath()).fileName()));
        }
        return documents;
    }

    template <typename T>
    static void calculateTrigrams(std::vector<Document> &documents,
                                  TaskContext<T> *context)
    {

#ifdef PARALLEL_INDEX
#pragma omp parallel for
#endif
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context->stopFlag) {
                continue;
            }
            unwrapTrigrams(documents[i]);
            std::invoke(context->callOnSuccess, context->caller);
        }
    }

    static void coutTime(decltype(std::chrono::steady_clock::now()) start,
                         std::string msg = "");

    template <typename T>
    void setUpDocuments(std::vector<Document> &documents,
                        TaskContext<T> *context)
    {
        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < documents.size(); ++i) {
            if (documents[i].trigramOccurrences.size() > 0) {
                this->documents.push_back(std::move(documents[i]));
            } else {
                std::invoke(context->callOnSuccess, context->caller);
            }
        }
        start = std::chrono::steady_clock::now();
        QSet<size_t> set;
        for (size_t i = 0; i < this->documents.size(); ++i) {
            if (context->stopFlag) {
                return;
            }
            for (auto &&it : this->documents[i].trigramOccurrences) {
                this->trigramsInFiles[it].insert(i);
            }
            std::invoke(context->callOnSuccess, context->caller);
        }
    }

    template <typename T>
    std::vector<size_t> getCandidateFileIds(std::string const &target,
                                            TaskContext<T> *context)
    {
        std::unordered_set<Trigram, Trigram::TrigramHash> targetTrigrams;
        if (target.size() < 3) {
            std::unordered_set<size_t> files;
            for (auto &pair : trigramsInFiles) {
                if (context->stopFlag) {
                    return {};
                }
                if (pair.first.substr(target)) {
                    files.insert(pair.second.begin(), pair.second.end());
                }
            }
            std::vector<size_t> result;
            result.insert(result.end(), files.begin(), files.end());
            return result;
        }
        // TODO: Unicode selections

        for (size_t i = 0; i < target.size() - 2 && !(context->stopFlag); ++i) {
            targetTrigrams.insert({&target.c_str()[i]});
        }
        if (context->stopFlag) {
            return {};
        }
        if (trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
            return {};
        }
        QSet<size_t> neccesaryFiles =
            trigramsInFiles.at(*targetTrigrams.begin());

        for (Trigram const &trigram : targetTrigrams) {
            if (context->stopFlag) {
                return {};
            }
            if (trigramsInFiles.count(trigram) > 0) {
                mergeUnorderedSets(neccesaryFiles, trigramsInFiles.at(trigram));
            } else {
                continue;
            }
        }
        std::vector<size_t> result;
        for (auto &&it : neccesaryFiles) {
            result.push_back(it);
        }
        return result;
    }

    template <typename T, typename R>
    void findOccurrencesInFiles(std::vector<size_t> fileIds,
                                std::string const &target,
                                TaskContext<T, R> *context)
    {
        std::boyer_moore_searcher processed_target(target.begin(),
                                                   target.end());
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (context->stopFlag) {
                continue;
            }
            auto vec =
                findExactOccurrences(documents[fileIds[i]], processed_target,
                                     target.size(), context);
            if (vec.size() > 0) {
                SubstringOccurrence occurrence{documents[fileIds[i]].filename,
                                               vec};
                std::invoke(context->callOnSuccess, context->caller,
                            occurrence);
            }
        }
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
#ifdef USE_BOYER_MOORE
        std::string::iterator occurrencePosition =
            std::search(buf.begin(), buf.end(), target);
        while (occurrencePosition != buf.end()) {
            result.push_back(occurrencePosition.base() - buf.begin().base());
            occurrencePosition =
                std::search(++occurrencePosition, buf.end(), target);
        }
#else
        for (size_t i = 0; i < blockSize - target.size() + 1; ++i) {
            if (memcmp(&buf[i], &target[0], target.size()) == 0) {
                result.push_back(i);
            }
        }
#endif
        size_t passed = blockSize;
        while (fileSize > 0 && !context->stopFlag) {
            size_t receivedBytes =
                fileInstance.read(&buf[blockSize], blockSize);
            fileSize -= receivedBytes;

#ifdef USE_BOYER_MOORE
            occurrencePosition = std::search(
                buf.begin() + blockSize - target_size, buf.end(), target);
            while (occurrencePosition != buf.end()) {
                result.push_back(passed + occurrencePosition.base() -
                                 buf.begin().base() - target_size);
                occurrencePosition =
                    std::search(++occurrencePosition, buf.end(), target);
            }
#else
            for (size_t i = blockSize - target.size() + 1;
                 i < blockSize + receivedBytes - target.size(); ++i) {
                if (memcmp(&buf[i], &target[0], target.size()) == 0) {
                    result.push_back(i + passed - target.size());
                }
            }
#endif
            passed += blockSize;
            memcpy(&buf[0], &buf[blockSize], blockSize);
        }
        coutTime(time, "read");
        fileInstance.close();
        return result;
    }

    static void mergeUnorderedSets(QSet<size_t> &destination,
                                   QSet<size_t> const &source);
    std::vector<SubstringOccurrence> findSubstring(QString const &);

    void reprocessFile(QString const &filename);
    const std::vector<Document> &getDocuments() const;

    void flush();

private:
    bool valid;

    void nothing();
    static const qint32 BUF_SIZE = 1 << 20;
    void catchSubstring(SubstringOccurrence const &);
    // TODO: ifdef test
    std::vector<SubstringOccurrence> storage;
    std::vector<Document> documents;
    std::unordered_map<Trigram, QSet<size_t>, Trigram::TrigramHash>
        trigramsInFiles;

    static void unwrapTrigrams(TrigramIndex::Document &document);
};
#endif // TRIGRAM_INDEX_H
