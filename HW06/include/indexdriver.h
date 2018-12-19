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
    void indexateAsync(QString const &path);
    void findSubstringAsync(QString const &substring);

private slots:
    void processChangedFile(const QString &);
    void processChangedDirectory(const QString &);

private:
    void indexSingular(QString const &path);
    void findSubstringSingular(QString const &substring);
    void sortD(Document &document);

    QFutureWatcher<void> globalTaskWatcher;
    QFutureWatcher<void> currentTaskWatcher;
    TrigramIndex index;
    std::atomic_size_t transactionalId;
    QFileSystemWatcher watcher;
};

#endif // INDEXWORKER_H
