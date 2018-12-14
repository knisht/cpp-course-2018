#include "include/indexworker.h"
#include <QDebug>
#include <QElapsedTimer>
#include <unordered_set>

IndexWorker::IndexWorker(QObject *parent) : QObject(parent), index()
{
    connect(&watcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
    connect(&watcher, SIGNAL(directoryChanged(const QString &)), this,
            SLOT(processChangedDirectory(const QString &)));
}

void IndexWorker::processChangedFile(const QString &path)
{
    index.reprocessFile(path);
    // TODO: emit render text
}

void IndexWorker::processChangedDirectory(const QString &path)
{
    qInfo() << "Directory changes detected in" << path;
    auto vec = index.reprocessDirectory(path);
    for (auto filename : vec) {
        watcher.addPath(filename);
    }
    // TODO: emit render text
}

void IndexWorker::catchOccurrence(SubstringOccurrence const &occurrence)
{
    // TODO: make light occurrence;
    SubstringOccurrence oc;
    oc.filename = occurrence.filename;
    emit occurrenceFound(occurrence);
}

void IndexWorker::findSubstring(QString const &substring)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexWorker, const SubstringOccurrence &> senderContext{
        validTransactionalId, this, &IndexWorker::catchOccurrence};
    TaskContext<IndexWorker, qsizetype> currentContext{
        validTransactionalId, this, &IndexWorker::increaseProgress};
    emit startedFinding();
    std::string stdTarget = substring.toStdString();
    QElapsedTimer timer;
    timer.start();
    auto fileIds = index.getCandidateFileIds(stdTarget, &currentContext);
    if (fileIds.size() == 0) {
        emit finishedFinding("");
        return;
    }
    emit determinedFilesAmount(static_cast<long long>(fileIds.size()));
    // TODO: just first occurr is ok, exat places are matter only if user wants
    // them
    std::vector<SubstringOccurrence> occs =
        index.findOccurrencesInFiles(fileIds, stdTarget, &senderContext);
    qInfo() << "Finding of" << substring << "finished in" << timer.elapsed()
            << "ms";
    for (auto &&occ : occs) {
        if (currentContext.isTaskCancelled()) {
            return;
        }
        emit catchOccurrence(occ);
    }

    if (currentContext.isTaskCancelled()) {
        emit finishedFinding("interrupted");
    } else {
        emit finishedFinding("");
    }
}

void IndexWorker::increaseProgress(qsizetype delta)
{
    emit progressChanged(delta);
}

void IndexWorker::watchDirectory(QString const &directory)
{
    watcher.addPath(directory);
}

void IndexWorker::indexate(QString const &path)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexWorker, qsizetype> currentContext{
        validTransactionalId, this, &IndexWorker::increaseProgress};
    TaskContext<IndexWorker, QString const &> directoryContext{
        validTransactionalId, this, &IndexWorker::watchDirectory};
    watcher.addPath(path);
    index.flush();
    currentDir = path;
    emit startedIndexing();
    QElapsedTimer timer;
    timer.start();
    auto documents =
        TrigramIndex::getFileEntries(path, &currentContext, &directoryContext);
    emit determinedFilesAmount(static_cast<long long>(documents.size()) * 2);
    TrigramIndex::calculateTrigrams(documents, &currentContext);
    index.setUpDocuments(documents, &currentContext);
    for (auto document : index.getDocuments()) {
        watcher.addPath(document.filename);
        emit progressChanged(1);
    }
    qInfo() << "Indexing of" << path << "finished in" << timer.elapsed()
            << "ms";
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
}

size_t IndexWorker::getTransactionalId() { return transactionalId; }

void IndexWorker::interrupt() { ++transactionalId; }
