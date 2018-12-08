#ifndef TRIGRAM_INDEX_H
#define TRIGRAM_INDEX_H

#include <QEventLoop>
#include <QString>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <functional>
#include <QDirIterator>
#include "trigram.h"

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

template <typename Caller>
struct TaskContext {
    bool stopFlag;
    Caller* caller;
    void (Caller::*callOnSuccess)();
};

class TrigramIndex
{
public:
    TrigramIndex();
    std::vector<SubstringOccurrence> findSubstring(QString const &target) const;
    void printDocuments();


    struct Document {
        QString filename;
        std::unordered_map<Trigram, std::vector<size_t>, Trigram::TrigramHash>
            trigramOccurrences;
        Document(QString filename) : filename(filename), trigramOccurrences{} {}
    };

    template <typename T>
    static std::vector<Document> getFileEntries(QString const &root, TaskContext<T> *context)
    {
        QDirIterator dirIterator(root, QDir::NoFilter,
                                 QDirIterator::Subdirectories);
        std::vector<TrigramIndex::Document> documents;
        while (dirIterator.hasNext() && !context->stopFlag) {
            dirIterator.next();
            if (dirIterator.fileInfo().isDir()) {
                continue;
            }
            documents.push_back(
                TrigramIndex::Document(QFile(dirIterator.filePath()).fileName()));
        }
        return documents;
    }

    template <typename T>
    static void calculateTrigrams(std::vector<Document>& documents, TaskContext<T> *context) {
        for (size_t i = 0; i < documents.size(); ++i) {
            if (context->stopFlag) {
                continue;
            }
            unwrapTrigrams(documents[i]);
            (context->caller->*(context->callOnSuccess))();
        }
    }

    template <typename T>
    void setUpDocuments(std::vector<Document> const& documents, TaskContext<T> *context) {
        for (size_t i = 0; i < documents.size(); ++i) {
            if (documents[i].trigramOccurrences.size() > 0) {
                this->documents.push_back(documents[i]);
                if (context->stopFlag) {
                    break;
                }
                for (auto &pair : documents[i].trigramOccurrences) {
                    this->trigramsInFiles[pair.first].push_back(i);
                }
            }
            (context->caller->*(context->callOnSuccess))();
        }
    }
//    static std::vector<size_t> findExactOccurrences(Document const &doc,
//                                             std::string const &target);
//    static void mergeVectorToList(std::list<size_t> &destination,
//                           std::vector<size_t> const &source);
private:

    bool valid;

    // TODO: put documents on disk
    std::vector<Document> documents;

    std::unordered_map<Trigram, std::vector<size_t>, Trigram::TrigramHash>
        trigramsInFiles;
    static void unwrapTrigrams(TrigramIndex::Document &document);
//    std::vector<SubstringOccurrence>
//    smallStringProcess(std::string const &target) const;
};
#endif // TRIGRAM_INDEX_H
