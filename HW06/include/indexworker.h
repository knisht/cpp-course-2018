#ifndef INDEXWORKER_H
#define INDEXWORKER_H

#include "TrigramIndex.h"
#include <QObject>

class IndexWorker : public QObject
{
    Q_OBJECT
public:
    explicit IndexWorker(QObject *parent = nullptr);

    void setUpIndex(QString const &root);

    void interrupt();

    void increaseProgress();

signals:
    void startedIndexing();
    void finishedIndexing(QString const&);
    void startedFinding();
    void finishedFinding(QString const&);
    void occurrenceFound(SubstringOccurrence const &);
    void determinedFilesAmount(qint64 filesAmount);
    void progressChanged(qint64 amount);

public slots:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);

private:
    TrigramIndex index;
    bool needInterrupt;
    TaskContext<IndexWorker> context;
};

#endif // INDEXWORKER_H
