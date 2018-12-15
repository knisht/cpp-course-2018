#include "include/indexdriver.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrentDepends>
#include <QtConcurrent/QtConcurrentMap>
#include <functional>
#include <unistd.h>
#include <unordered_set>

IndexDriver::IndexDriver(QObject *parent)
    : QObject(parent), futureWatcher(), index(), transactionalId(0), watcher()
{
    connect(&watcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
    connect(&watcher, SIGNAL(directoryChanged(const QString &)), this,
            SLOT(processChangedDirectory(const QString &)));
}

IndexDriver::~IndexDriver()
{
    ++transactionalId;
    futureWatcher.cancel();
}

void IndexDriver::processChangedFile(const QString &path)
{
    //    index.reprocessFile(path);
}

void IndexDriver::processChangedDirectory(const QString &path)
{
    //    qInfo() << "Directory changes detected in" << path;
    //    auto vec = index.reprocessDirectory(path);
    //    for (auto filename : vec) {
    //        watcher.addPath(filename);
    //    }
}

void IndexDriver::catchProperFile(QString const &occurrence)
{
    emit properFileFound(occurrence);
}

void IndexDriver::findSubstring(QString const &substring)
{
    interrupt();
    QFuture<void> indexFuture =
        QtConcurrent::run(this, &IndexDriver::findSubstringSingular, substring);
    futureWatcher.setFuture(indexFuture);
}

void IndexDriver::findSubstringSingular(QString const &substring)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexDriver, qsizetype> currentContext{
        validTransactionalId, this, &IndexDriver::increaseProgress};
    TaskContext<IndexDriver, QString const &> catcherContext{
        validTransactionalId, this, &IndexDriver::catchProperFile};
    emit startedFinding();
    std::string stdTarget = substring.toStdString();
    QElapsedTimer timer;
    timer.start();
    std::vector<size_t> fileIds =
        index.getCandidateFileIds(stdTarget, currentContext);
    if (fileIds.size() == 0) {
        emit finishedFinding("");
        return;
    }
    emit determinedFilesAmount(static_cast<long long>(fileIds.size()));
    index.findOccurrencesInFiles(fileIds, stdTarget, catcherContext);
    qInfo() << "Finding of" << substring << "finished in" << timer.elapsed()
            << "ms";
    if (currentContext.isTaskCancelled()) {
        emit finishedFinding("interrupted");
    } else {
        emit finishedFinding("");
    }
}

void IndexDriver::increaseProgress(qsizetype delta)
{
    emit progressChanged(delta);
}

void IndexDriver::setWatchingDirectory(QString const &directory)
{
    //    watcher.addPath(directory);
}

void IndexDriver::indexate(QString const &path)
{
    interrupt();
    QFuture<void> indexFuture =
        QtConcurrent::run(this, &IndexDriver::indexSingular, path);
    futureWatcher.setFuture(indexFuture);
}

// TODO: better naming
void IndexDriver::indexSingular(QString const &path)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexDriver, qsizetype> currentContext{
        validTransactionalId, this, &IndexDriver::increaseProgress};
    TaskContext<IndexDriver, QString const &> directoryContext{
        validTransactionalId, this, &IndexDriver::setWatchingDirectory};
    //    watcher.addPath(path);
    index.flush();
    emit startedIndexing();
    QElapsedTimer timer;
    timer.start();
    std::vector<Document> documents =
        TrigramIndex::getFileEntries(path, directoryContext);
    emit determinedFilesAmount(static_cast<long long>(documents.size()));
    qDebug() << documents.size();
    using namespace std::placeholders;
    auto activatedUnwrapTrigrams = std::bind(
        TrigramIndex::unwrapTrigrams<IndexDriver>, _1, currentContext);
    //    TrigramIndex::calculateTrigrams(documents, currentContext);
    QtConcurrent::blockingMap(documents.begin(), documents.end(),
                              activatedUnwrapTrigrams);
    index.getFilteredDocuments(documents);
    QtConcurrent::blockingMap(index.documents, &Document::sort);
    //        for (Document const &document :
    //    index.getDocuments()) {
    //        watcher.addPath(document.filename);
    //        emit increaseProgress(1);
    //    }
    qInfo() << "Indexing of" << path << "finished in" << timer.elapsed()
            << "ms";
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
}

size_t IndexDriver::getTransactionalId() { return transactionalId; }

std::vector<size_t> IndexDriver::getFileStat(QString const &filename,
                                             QString const &pattern)
{
    std::string preprocessedPattern = pattern.toStdString();
    std::boyer_moore_searcher searcher(preprocessedPattern.begin(),
                                       preprocessedPattern.end());
    TaskContext<IndexDriver, QString const &> context = {
        transactionalId, this, &IndexDriver::catchProperFile};
    return index.findExactOccurrences(filename, searcher,
                                      preprocessedPattern.size(), context);
}

void IndexDriver::interrupt()
{
    ++transactionalId;
    futureWatcher.cancel();
    if (futureWatcher.isRunning()) {
        futureWatcher.waitForFinished();
    }
}
