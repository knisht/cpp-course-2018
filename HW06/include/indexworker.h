#ifndef INDEXWORKER_H
#define INDEXWORKER_H

#include "TrigramIndex.h"
#include <QFileSystemWatcher>
#include <QObject>

class IndexWorker : public QObject
{
    Q_OBJECT
public:
    explicit IndexWorker(QObject *parent = nullptr);

    void setUpIndex(QString const &root);

    void interrupt();

    void increaseProgress();
    void catchOccurrence(SubstringOccurrence const &);

signals:
    void startedIndexing();
    void finishedIndexing(QString const &);
    void startedFinding();
    void finishedFinding(QString const &);
    void occurrenceFound(SubstringOccurrence const &);
    void determinedFilesAmount(qint64 filesAmount);
    void progressChanged(qint64 amount);

public slots:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);

private slots:
    void processChangedFile(const QString &);

private:
    TrigramIndex index;
    QDir currentDir;
    QFileSystemWatcher watcher;
    TaskContext<IndexWorker> context;
    TaskContext<IndexWorker, const SubstringOccurrence &> senderContext;
};

#endif // INDEXWORKER_H
