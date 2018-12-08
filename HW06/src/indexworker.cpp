#include "include/indexworker.h"
#include <QDebug>
#include <unordered_set>

IndexWorker::IndexWorker(QObject *parent)
    : QObject(parent), index()
{
    context.stopFlag = false;
    context.caller = this;
    context.callOnSuccess = &IndexWorker::increaseProgress;
}


void IndexWorker::findSubstring(QString const &substring)
{
    index.printDocuments();
//    needInterrupt = false;
//    emit startedFinding();
//    qDebug() << substring << " <-- search";
//    std::string stdTarget = substring.toStdString();
//    if (stdTarget.size() <= 2) {
//        auto result = index.smallStringProcess(stdTarget);
//        for (auto&& occurrence : result) {
//            // TODO: small file online emit
//            emit occurrenceFound(occurrence);
//        }
//        if (needInterrupt) {
//            emit finishedFinding("interrupted");
//        } else {
//            emit finishedFinding("");
//        }
//        return;
//    }
//    std::unordered_set<TrigramIndex::Trigram, TrigramIndex::TrigramHash> targetTrigrams;
//    for (size_t i = 0; i < stdTarget.size() - 2; ++i) {
//        targetTrigrams.insert({&stdTarget.c_str()[i]});
//    }
//    std::list<size_t> neccesaryFiles;
//    if (index.trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
//        if (needInterrupt) {
//            emit finishedFinding("interrupted");
//        } else {
//            emit finishedFinding("");
//        }
//        return;
//    }
//    for (size_t fileId : index.trigramsInFiles.at(*targetTrigrams.begin())) {
//        neccesaryFiles.push_back(fileId);
//    }
//    for (TrigramIndex::Trigram const &trigram : targetTrigrams) {
//        if (index.trigramsInFiles.count(trigram) > 0) {
//            TrigramIndex::mergeVectorToList(neccesaryFiles, index.trigramsInFiles.at(trigram));
//        }
//    }
//    emit determinedFilesAmount(neccesaryFiles.size());
//    std::vector<SubstringOccurrence> resultFiles;
//    for (size_t fileId : neccesaryFiles) {
//        if (needInterrupt) {
//            break;
//        }
//        emit occurrenceFound(
//            {index.documents[fileId].filename,
//             TrigramIndex::findExactOccurrences(index.documents[fileId], stdTarget)});
//    }
//    if (needInterrupt) {
//        emit finishedFinding("interrupted");
//    } else {
//        emit finishedFinding("");
//    }
}

void IndexWorker::increaseProgress() {
    emit progressChanged(1);
}

void IndexWorker::indexate(QString const &path)
{
    context.stopFlag = false;
    emit startedIndexing();
    qDebug() << "index started";
    auto documents = TrigramIndex::getFileEntries(path, &context);
    emit determinedFilesAmount(static_cast<long long>(documents.size()) * 2);
    qDebug() << documents.size();
    TrigramIndex::calculateTrigrams(documents, &context);
    index.setUpDocuments(documents, &context);
    qDebug() << "indexing ended";
    if (context.stopFlag) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
}

void IndexWorker::interrupt() {
    qDebug() << "interrupting...";
    context.stopFlag = true;
}
