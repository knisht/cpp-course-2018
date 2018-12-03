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

    void setDir(QString const &dir);
signals:
    void startedIndexing();
    void finishedIndexing();
    void startedFinding();
    void finishedFinding();
    void occurrenceFound(SubstringOccurrence);

public slots:
    void indexate(QString const &path);
    // TODO: online fetch
    void findSubstring(QString const &substring);

private:
    TrigramIndex index;
    QString currentDir;
};

#endif // INDEXWORKER_H
