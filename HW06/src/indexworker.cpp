#include "include/indexworker.h"
#include <QDebug>
#include <unordered_set>

IndexWorker::IndexWorker(QObject *parent) : QObject(parent), index()
{
    connect(&watcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
    connect(this, SIGNAL(testSignal()), this, SLOT(testSlot()));
}

void IndexWorker::processChangedFile(const QString &path)
{
    index.reprocessFile(path);
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

void IndexWorker::testSlot() { qDebug() << "test slot agred!"; }

void IndexWorker::indexate(QString const &path)
{
    ++transactionalId;
    TaskContext<IndexWorker, qsizetype> currentContext{
        transactionalId, this, &IndexWorker::increaseProgress};
    // TODO: push watcher to taskContext
    for (auto document : index.getDocuments()) {
        watcher.removePath(document.filename);
    }
    index.flush();
    currentDir = path;
    emit startedIndexing();
    auto documents = TrigramIndex::getFileEntries(path, &currentContext);
    emit determinedFilesAmount(static_cast<long long>(documents.size()) * 2);
    TrigramIndex::calculateTrigrams(documents, &currentContext);
    index.setUpDocuments(documents, &currentContext);
    for (auto document : index.getDocuments()) {
        watcher.addPath(document.filename);
    }
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
}

size_t IndexWorker::getTransactionalId() { return transactionalId; }

void IndexWorker::interrupt()
{
    qDebug() << "interrupting...";
    ++transactionalId;
    emit testSignal();
}
