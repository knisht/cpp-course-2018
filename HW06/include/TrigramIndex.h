#ifndef TRIGRAM_INDEX_H
#define TRIGRAM_INDEX_H

#include "trigram.h"
#include <QDebug>
#include <QDirIterator>
#include <QEventLoop>
#include <QString>
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
        Document(QString filename) : filename(filename), trigramOccurrences{} {}
    };

    template <typename T>
    static std::vector<Document> getFileEntries(QString const &root,
                                                TaskContext<T> *context)
    {
        QDirIterator dirIterator(root, QDir::NoFilter,
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
#pragma omp parallel for
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context->stopFlag) {
                continue;
            }
            unwrapTrigrams(documents[i]);
            std::invoke(context->callOnSuccess, context->caller);
        }
    }

    template <typename T>
    void setUpDocuments(std::vector<Document> const &documents,
                        TaskContext<T> *context)
    {
        for (size_t i = 0; i < documents.size(); ++i) {
            if (documents[i].trigramOccurrences.size() > 0) {
                this->documents.push_back(documents[i]);
                if (context->stopFlag) {
                    break;
                }
                for (auto &trigram : documents[i].trigramOccurrences) {
                    this->trigramsInFiles[trigram].push_back(
                        this->documents.size() - 1);
                }
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
            qDebug() << "Here!";
            std::unordered_set<size_t> files;
            for (auto &pair : trigramsInFiles) {
                if (context->stopFlag) {
                    break;
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

        for (size_t i = 0; i < target.size() - 2; ++i) {
            targetTrigrams.insert({&target.c_str()[i]});
        }
        if (trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
            return {};
        }
        std::list<size_t> neccesaryFiles;
        for (size_t fileId : trigramsInFiles.at(*targetTrigrams.begin())) {
            neccesaryFiles.push_back(fileId);
        }

        for (Trigram const &trigram : targetTrigrams) {
            if (trigramsInFiles.count(trigram) > 0 || context->stopFlag) {
                mergeVectorToList(neccesaryFiles, trigramsInFiles.at(trigram));
            } else {
                return {};
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
        //#pragma omp parallel for
        for (size_t fileId : fileIds) {
            if (context->stopFlag) {
                break;
            }
            auto vec = findExactOccurrences(documents[fileId], target);
            if (vec.size() > 0) {
                SubstringOccurrence occurrence{documents[fileId].filename, vec};
                std::invoke(context->callOnSuccess, context->caller,
                            occurrence);
            }
        }
    }
    static std::vector<size_t> findExactOccurrences(Document const &doc,
                                                    std::string const &target);
    static void mergeVectorToList(std::list<size_t> &destination,
                                  std::vector<size_t> const &source);
    std::vector<SubstringOccurrence> findSubstring(QString const &);

private:
    bool valid;

    void nothing();
    void catchSubstring(SubstringOccurrence const &);
    // TODO: ifdef debug
    std::vector<SubstringOccurrence> storage;
    // TODO: put documents on disk
    std::vector<Document> documents;
    std::unordered_map<Trigram, std::vector<size_t>, Trigram::TrigramHash>
        trigramsInFiles;

    static void unwrapTrigrams(TrigramIndex::Document &document);
};
#endif // TRIGRAM_INDEX_H
