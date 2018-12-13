#ifndef INDEXWORKER_H
#define INDEXWORKER_H

#include "trigramindex.h"
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
    void setString(QString const &);
    size_t getTransactionalId();

signals:
    void startedIndexing();
    void finishedIndexing(QString const &);
    void startedFinding();
    void finishedFinding(QString const &);
    void occurrenceFound(SubstringOccurrence const &);
    void determinedFilesAmount(qint64 filesAmount);
    void progressChanged(qint64 amount);
    void testSignal();

public slots:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);

private slots:
    void processChangedFile(const QString &);
    void testSlot();

private:
    TrigramIndex index;
    QDir currentDir;
    bool working;
    std::atomic_size_t transactionalId;
    QFileSystemWatcher watcher;
};

#endif // INDEXWORKER_H
