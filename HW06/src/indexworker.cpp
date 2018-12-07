#include "include/indexworker.h"
#include <QDebug>
#include <unordered_set>

IndexWorker::IndexWorker(QObject *parent)
    : QObject(parent), index()
{
}

void IndexWorker::setUpIndex(QString const &root) { index.setUp(root); }

void IndexWorker::findSubstring(QString const &substring)
{
    needInterrupt = false;
    emit startedFinding();
    qDebug() << substring << " <-- search";
    std::string stdTarget = substring.toStdString();
    if (stdTarget.size() <= 2) {
        auto result = index.smallStringProcess(stdTarget);
        for (auto&& occurrence : result) {
            // TODO: small file online emit
            emit occurrenceFound(occurrence);
        }
        if (needInterrupt) {
            emit finishedFinding("interrupted");
        } else {
            emit finishedFinding("");
        }
        return;
    }
    std::unordered_set<TrigramIndex::Trigram, TrigramIndex::TrigramHash> targetTrigrams;
    for (size_t i = 0; i < stdTarget.size() - 2; ++i) {
        targetTrigrams.insert({&stdTarget.c_str()[i]});
    }
    std::list<size_t> neccesaryFiles;
    if (index.trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
        if (needInterrupt) {
            emit finishedFinding("interrupted");
        } else {
            emit finishedFinding("");
        }
        return;
    }
    for (size_t fileId : index.trigramsInFiles.at(*targetTrigrams.begin())) {
        neccesaryFiles.push_back(fileId);
    }
    for (TrigramIndex::Trigram const &trigram : targetTrigrams) {
        if (index.trigramsInFiles.count(trigram) > 0) {
            TrigramIndex::mergeVectorToList(neccesaryFiles, index.trigramsInFiles.at(trigram));
        }
    }
    emit determinedFilesAmount(neccesaryFiles.size());
    std::vector<SubstringOccurrence> resultFiles;
    for (size_t fileId : neccesaryFiles) {
        if (needInterrupt) {
            break;
        }
        emit occurrenceFound(
            {index.documents[fileId].filename,
             TrigramIndex::findExactOccurrences(index.documents[fileId], stdTarget)});
    }
    if (needInterrupt) {
        emit finishedFinding("interrupted");
    } else {
        emit finishedFinding("");
    }
}

void IndexWorker::indexate(QString const &path)
{
    needInterrupt = false;
    emit startedIndexing();
    qDebug() << "index started";
    index.documents = TrigramIndex::getFileEntries(path);
    emit determinedFilesAmount(index.documents.size() * 2);
    qDebug() << index.documents.size();
#pragma omp parallel for
    for (size_t i = 0; i < index.documents.size(); ++i) {
        if (needInterrupt) {
            continue;
            // TODO: make warning about interrupting
        }
        TrigramIndex::unwrapTrigrams(index.documents[i]);
        qDebug() << index.documents[i].filename << index.documents[i].trigramOccurrences.size();
        emit progressChanged(1);
    }
    for (size_t i = 0; i < index.documents.size(); ++i) {
        if (needInterrupt) {
            break;
        }
        for (auto &pair : index.documents[i].trigramOccurrences) {
            index.trigramsInFiles[pair.first].push_back(i);
        }
        emit progressChanged(1);
    }
    qDebug() << "indexing ended";
    if (needInterrupt) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
}

void IndexWorker::interrupt() {
    qDebug() << "interrupting...";
    needInterrupt = true;
}
