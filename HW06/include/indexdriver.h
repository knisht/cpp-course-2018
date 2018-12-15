#ifndef INDEXWORKER_H
#define INDEXWORKER_H

#include "trigramindex.h"
#include <QFileSystemWatcher>
#include <QObject>
#include <QtConcurrent/QtConcurrent>

class IndexDriver : public QObject
{
    Q_OBJECT
public:
    explicit IndexDriver(QObject *parent = nullptr);

    IndexDriver(IndexDriver const &) = delete;
    IndexDriver &operator=(IndexDriver const &) = delete;

    void interrupt();
    void increaseProgress(qsizetype);
    void setWatchingDirectory(QString const &directory);
    void catchProperFile(QString const &);
    std::vector<size_t> getFileStat(QString const &filename,
                                    QString const &pattern);
    size_t getTransactionalId();

    ~IndexDriver();
signals:
    void startedIndexing();
    void finishedIndexing(QString const &exitMessage);
    void startedFinding();
    void finishedFinding(QString const &exitMessage);
    void properFileFound(QString const &);
    void determinedFilesAmount(qint64 filesAmount);
    void progressChanged(qint64 amount);

public slots:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);

private slots:
    void processChangedFile(const QString &);
    void processChangedDirectory(const QString &);

private:
    void indexSingular(QString const &path);
    void findSubstringSingular(QString const &substring);

    QFutureWatcher<void> futureWatcher;
    TrigramIndex index;
    std::atomic_size_t transactionalId;
    QFileSystemWatcher watcher;
};

#endif // INDEXWORKER_H
