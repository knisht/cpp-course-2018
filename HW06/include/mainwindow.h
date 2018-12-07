#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexworker.h"
#include <QDebug>
#include <QListWidget>
#include <QMainWindow>
#include <QThread>
#include <QTextCursor>

namespace Ui
{

class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void findSubstring();
    void getFileContent(QListWidgetItem *);
    void changeDirectory();
    void nextOccurrence();
    void previousOccurrence();
    void getOccurrence(SubstringOccurrence const &);
    void setProgressBarLimit(qint64 limit);
    void changeProgressBarValue(qint64 delta);
    void stopActions();

private slots:
    void onStartedIndexing();
    void onFinishedIndexing(QString const&);
    void onStartedFinding();
    void onFinishedFinding(QString const&);

signals:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);
    void interrupt();

private:

    struct CursorPosition {
        int occurrenceIndex;
        SubstringOccurrence* document;
    } cursor;

    Ui::MainWindow *ui;
    QString currentDir;
    std::vector<SubstringOccurrence> currentOccurrences;
    size_t wordSize;
    QThread thread;
    QTextCursor defaultCursor;
    IndexWorker worker;
};

#endif // MAINWINDOW_H
