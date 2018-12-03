#include "include/indexworker.h"
#include <QDebug>

IndexWorker::IndexWorker(QObject *parent)
    : QObject(parent), index(), currentDir("../../")
{
}

void IndexWorker::setUpIndex(QString const &root) { index.setUp(root); }

void IndexWorker::findSubstring(QString const &substring)
{
    emit startedFinding();
    qDebug() << substring << " <-- search";
    auto occurrenceVector = index.findSubstring(substring);
    for (auto &&it : occurrenceVector) {
        emit occurrenceFound(it);
    }
    emit finishedFinding();
}

void IndexWorker::setDir(QString const &dir) { currentDir = dir; }

void IndexWorker::indexate(QString const &path)
{
    emit startedIndexing();
    setUpIndex(path);
    qDebug() << "indexing ended";
    emit finishedIndexing();
}
