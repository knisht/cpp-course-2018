#include "include/indexworker.h"
#include <QDebug>
#include <unordered_set>

IndexWorker::IndexWorker(QObject *parent) : QObject(parent), index()
{
    context.stopFlag = false;
    context.caller = this;
    context.callOnSuccess = &IndexWorker::increaseProgress;
    senderContext.stopFlag = false;
    senderContext.caller = this;
    senderContext.callOnSuccess = &IndexWorker::catchOccurrence;
}

void IndexWorker::catchOccurrence(SubstringOccurrence const &occurrence)
{
    emit occurrenceFound(occurrence);
}

void IndexWorker::findSubstring(QString const &substring)
{
    context.stopFlag = false;
    senderContext.stopFlag = false;
    emit startedFinding();
    qDebug() << substring << " <-- search";
    std::string stdTarget = substring.toStdString();
    auto fileIds = index.getCandidateFileIds(stdTarget, &context);
    if (fileIds.size() == 0) {
        emit finishedFinding("");
        return;
    }
    qDebug() << "found ids!";
    emit determinedFilesAmount(static_cast<long long>(fileIds.size()));
    // TODO: just first occurr is ok, exat places are matter only if user wants
    // them
    index.findOccurrencesInFiles(fileIds, stdTarget, &senderContext);

    if (context.stopFlag) {
        emit finishedFinding("interrupted");
    } else {
        emit finishedFinding("");
    }
}

void IndexWorker::increaseProgress() { emit progressChanged(1); }

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

void IndexWorker::interrupt()
{
    qDebug() << "interrupting...";
    context.stopFlag = true;
    senderContext.stopFlag = true;
}
