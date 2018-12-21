#ifndef LIBRARIAN_INDEXDRIVER_H
#define LIBRARIAN_INDEXDRIVER_H

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
    void catchProperFile(QString const &, size_t sender_id);
    std::vector<size_t> getFileStat(QString const &filename,
                                    QString const &pattern);
    size_t getTransactionalId();
    bool validate(SubstringOccurrence const &occurrence);

    ~IndexDriver();
signals:
    void startedIndexing();
    void finishedIndexing(QString const &exitMessage);
    void startedFinding();
    void finishedFinding(QString const &exitMessage);
    void properFileFound(SubstringOccurrence const &);
    void determinedFilesAmount(qint64 filesAmount);
    void progressChanged(qint64 amount);

public slots:
    void indexateAsync(QString const &path);
    void findSubstringAsync(QString const &substring);

private slots:
    void processChangedFile(const QString &);
    void processChangedDirectory(const QString &);

private:
    void indexateSync(QString const &path);
    void findSubstringSync(QString const &substring);

    QFutureWatcher<void> globalTaskWatcher;
    QFutureWatcher<void> currentTaskWatcher;
    TrigramIndex index;
    std::atomic_size_t transactionalId;
    QFileSystemWatcher watcher;
};

#endif // INDEXWORKER_H
