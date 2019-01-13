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
      fileWatcher(), isIndexing(false)
{
    connect(&fileWatcher, SIGNAL(fileChanged(const QString &)), this,
            SLOT(processChangedFile(const QString &)));
    connect(&fileWatcher, SIGNAL(directoryChanged(const QString &)), this,
            SLOT(processChangedDirectory(const QString &)));
}

IndexDriver::~IndexDriver() { interrupt(); }

void IndexDriver::processChangedFile(const QString &path)
{
    qInfo() << "File changes detected in" << path;
    TaskContext<IndexDriver, qsizetype> context{
        transactionalId, this, &IndexDriver::nothing<qsizetype>};
    index.reprocessFile(path, context);
    fileWatcher.addPath(path);
}

void IndexDriver::processChangedDirectory(const QString &path)
{
    if (isIndexing) {
        return;
    }
    qInfo() << "Directory changes detected in" << path;
    TaskContext<IndexDriver, qsizetype> context{
        transactionalId, this, &IndexDriver::nothing<qsizetype>};
    for (QString const &newEntry : index.reprocessDirectory(path, context)) {
        if (QFileInfo(newEntry).isDir()) {
            if (!fileWatcher.directories().contains(newEntry)) {
                processChangedDirectory(newEntry);
                fileWatcher.addPath(newEntry);
            }
        } else if (QFileInfo(newEntry).isFile()) {
            processChangedFile(newEntry);
        }
    }
}

void IndexDriver::catchProperFile(QString const &occurrence, size_t sender_id)
{
    if (sender_id == transactionalId) {
        SubstringOccurrence pair{occurrence, sender_id};
        emit properFileFound(pair);
    }
}

void IndexDriver::findSubstringAsync(QString const &substring,
                                     bool parallelSearch)
{
    interrupt();
    QFuture<void> indexFuture = QtConcurrent::run(
        this, &IndexDriver::findSubstringImpl, substring, parallelSearch);
    globalTaskWatcher.setFuture(indexFuture);
}

void IndexDriver::findSubstringImpl(QString const &substring,
                                    bool parallelSearch)
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
    std::vector<QString> filenames =
        index.getCandidateFileNames(stdTarget, currentContext);
    if (filenames.size() == 0) {
        emit finishedFinding("No files");
        return;
    }
    emit determinedFilesAmount(static_cast<long long>(filenames.size()));
    if (parallelSearch) {
        TrigramIndex::Searcher searcher(stdTarget);
        using namespace std::placeholders;
        auto preparedFunc = [&](QString filename) {
            index.findOccurrencesInFile(filename, searcher, catcherContext);
        };
        currentTaskWatcher.setFuture(QtConcurrent::map(
            filenames.begin(), filenames.end(), preparedFunc));
        currentTaskWatcher.waitForFinished();
    } else {
        index.findOccurrencesInFiles(filenames, stdTarget, catcherContext);
    }
    qInfo() << "Finding of" << substring << "finished in" << timer.elapsed()
            << "ms";
    if (currentContext.isTaskCancelled()) {
        emit finishedFinding("Interrupted");
    } else {
        emit finishedFinding("");
    }
}

void IndexDriver::increaseProgress(qsizetype delta)
{
    emit progressChanged(delta);
}

void IndexDriver::setWatchFile(QString const &file)
{
    fileWatcher.addPath(file);
}

void IndexDriver::indexateAsync(QString const &path, bool fileWatching)
{
    interrupt();
    QFuture<void> indexFuture =
        QtConcurrent::run(this, &IndexDriver::indexateImpl, path, fileWatching);
    globalTaskWatcher.setFuture(indexFuture);
}

void IndexDriver::indexateImpl(QString const &path, bool fileWatching)
{
    size_t validTransactionalId = ++transactionalId;
    TaskContext<IndexDriver, qsizetype> currentContext{
        validTransactionalId, this, &IndexDriver::increaseProgress};
    TaskContext<IndexDriver, QString const &> directoryContext{
        validTransactionalId, this,
        (fileWatching ? &IndexDriver::setWatchFile
                      : &IndexDriver::nothing<QString const &>)};
    isIndexing = true;
    index.flush();
    emit startedIndexing();
    QElapsedTimer timer;
    timer.start();
    QStringList fileDump = fileWatcher.files();
    for (QString const &file : fileDump) {
        if (currentContext.isTaskCancelled()) {
            break;
        }
        fileWatcher.removePath(file);
    }
    std::vector<TrigramIndex::DocumentEntry> documents =
        TrigramIndex::getFileEntries(path, directoryContext);
    if (currentContext.isTaskCancelled()) {
        isIndexing = false;
        emit finishedIndexing("interrupted");
        return;
    }
    emit determinedFilesAmount(static_cast<long long>(documents.size() * 2));
    using namespace std::placeholders;
    auto activatedUnwrapTrigrams = [&](TrigramIndex::DocumentEntry &document) {
        TrigramIndex::unwrapTrigrams(document.first, document.second,
                                     currentContext);
    };
    currentTaskWatcher.setFuture(QtConcurrent::map(
        documents.begin(), documents.end(), activatedUnwrapTrigrams));
    currentTaskWatcher.waitForFinished();
    if (currentContext.isTaskCancelled()) {
        isIndexing = false;
        emit finishedIndexing("interrupted");
        return;
    }
    index.getFilteredDocuments(std::move(documents), currentContext);
    if (currentContext.isTaskCancelled()) {
        isIndexing = false;
        emit finishedIndexing("interrupted");
        return;
    }
    qInfo() << "Indexing of" << path << "finished in" << timer.elapsed()
            << "ms";
    if (currentContext.isTaskCancelled()) {
        emit finishedIndexing("interrupted");
    } else {
        emit finishedIndexing("");
    }
    isIndexing = false;
}

bool IndexDriver::validate(const SubstringOccurrence &occurrence)
{
    return transactionalId == occurrence.id;
}

size_t IndexDriver::getTransactionalId() { return transactionalId; }

std::vector<size_t> IndexDriver::getFileStat(QString const &filename,
                                             QString const &pattern)
{
    TaskContext<IndexDriver, QString const &, size_t> context = {
        transactionalId, this, &IndexDriver::catchProperFile};
    return index.collectAllOccurrences(filename, pattern.toStdString(),
                                       context);
}

void IndexDriver::interrupt()
{
    ++transactionalId;
    globalTaskWatcher.cancel();
    currentTaskWatcher.cancel();
    currentTaskWatcher.waitForFinished();
    globalTaskWatcher.waitForFinished();
}
