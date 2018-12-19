#include "include/indexdriver.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrentDepends>
#include <QtConcurrent/QtConcurrentMap>
#include <functional>
#include <unistd.h>
#include <unordered_set>

IndexDriver::IndexDriver(QObject *parent)
    : QObject(parent), globalTaskWatcher(), index(), transactionalId(0),
      watcher()
{
    connect(&watcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
    connect(&watcher, SIGNAL(directoryChanged(const QString &)), this,
            SLOT(processChangedDirectory(const QString &)));
}

IndexDriver::~IndexDriver() { interrupt(); }

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

void IndexDriver::catchProperFile(QString const &occurrence, size_t sender_id)
{
    if (sender_id == transactionalId) {
        emit properFileFound(occurrence);
    } else {
        qDebug() << "rejected";
    }
}

void IndexDriver::findSubstringAsync(QString const &substring)
{
    interrupt();
    QFuture<void> indexFuture =
        QtConcurrent::run(this, &IndexDriver::findSubstringSingular, substring);
    globalTaskWatcher.setFuture(indexFuture);
}

void IndexDriver::findSubstringSingular(QString const &substring)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexDriver, qsizetype> currentContext{
        validTransactionalId, this, &IndexDriver::increaseProgress};
    TaskContext<IndexDriver, QString const &, size_t> catcherContext{
        validTransactionalId, this, &IndexDriver::catchProperFile};
    emit startedFinding();
    std::string stdTarget = substring.toStdString();
    QElapsedTimer timer;
    timer.start();
    std::vector<size_t> fileIds =
        index.getCandidateFileIds(stdTarget, currentContext);
    if (fileIds.size() == 0) {
        emit finishedFinding("No files");
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

void IndexDriver::indexateAsync(QString const &path)
{
    interrupt();
    QFuture<void> indexFuture =
        QtConcurrent::run(this, &IndexDriver::indexSingular, path);
    globalTaskWatcher.setFuture(indexFuture);
}

void IndexDriver::sortD(Document &document)
{
    document.sort();
    emit progressChanged(1);
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
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
        return;
    }
    emit determinedFilesAmount(static_cast<long long>(documents.size()));
    qDebug() << "Documents:" << documents.size();
    using namespace std::placeholders;
    auto activatedUnwrapTrigrams = std::bind(
        TrigramIndex::unwrapTrigrams<IndexDriver>, _1, currentContext);
    QFuture<void> unwrapJob = QtConcurrent::map(
        documents.begin(), documents.end(), activatedUnwrapTrigrams);
    currentTaskWatcher.setFuture(unwrapJob);
    unwrapJob.waitForFinished();
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
        return;
    }
    index.getFilteredDocuments(documents, currentContext);
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
        return;
    }
    //    std::unordered_set<Trigram, Trigram::TrigramHash> trigrams;
    //    size_t sum = 0;
    //    size_t ascii_cnt = 0;
    //    for (Document const &d : index.getDocuments()) {
    //        sum += d.trigramOccurrences.size();
    //        for (Trigram t : d.trigramOccurrences) {
    //            trigrams.insert(t);
    //            std::string text = t.toString();
    //            if ((text[0] & (1 << 8)) == 0 && (text[2] & (1 << 8)) == 0 &&
    //                (text[1] & (1 << 8)) == 0) {
    //                ++ascii_cnt;
    //            }
    //        }
    //    }
    //    qDebug() << "all trigrams" << sum;
    //    qDebug() << "Different Trigrams" << trigrams.size();
    //    qDebug() << "Ascii from them" << ascii_cnt;
    //    auto processedSort = std::bind(&IndexDriver::sortD, this, _1);
    //    QFuture<void> sortJob = QtConcurrent::map(
    //        index.documents.begin(), index.documents.end(), processedSort);
    //    currentTaskWatcher.setFuture(sortJob);
    //    sortJob.waitForFinished();
    //        for (Document const &document :
    //    index.getDocuments()) {
    //        watcher.addPath(document.filename);
    //        emit increaseProgress(1);
    //    }
    qInfo() << "Indexing of" << path << "finished in" << timer.elapsed()
            << "ms";
    qDebug() << "filesize" << index.getDocuments().size();
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
    // TODO: move to trigramindex
    std::string preprocessedPattern = pattern.toStdString();
    std::boyer_moore_searcher searcher(preprocessedPattern.begin(),
                                       preprocessedPattern.end());
    TaskContext<IndexDriver, QString const &, size_t> context = {
        transactionalId, this, &IndexDriver::catchProperFile};
    return index.findExactOccurrences(filename, searcher,
                                      preprocessedPattern.size(), context);
}

void IndexDriver::interrupt()
{
    qDebug() << "Interrupting";
    ++transactionalId;
    globalTaskWatcher.cancel();
    currentTaskWatcher.cancel();
    currentTaskWatcher.waitForFinished();
    globalTaskWatcher.waitForFinished();
}
