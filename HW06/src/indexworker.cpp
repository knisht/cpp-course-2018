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
    connect(&watcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
}

void IndexWorker::processChangedFile(const QString &path)
{
    index.reprocessFile(path);
    // TODO: emit render text
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

    // TODO: push watcher to taskContext
    for (auto document : index.getDocuments()) {
        watcher.removePath(document.filename);
    }
    index.flush();
    currentDir = path;
    context.stopFlag = false;
    emit startedIndexing();
    qDebug() << "index started";
    auto documents = TrigramIndex::getFileEntries(path, &context);
    emit determinedFilesAmount(static_cast<long long>(documents.size()) * 2);
    qDebug() << documents.size();
    TrigramIndex::calculateTrigrams(documents, &context);
    index.setUpDocuments(documents, &context);
    for (auto document : index.getDocuments()) {
        watcher.addPath(document.filename);
    }
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
