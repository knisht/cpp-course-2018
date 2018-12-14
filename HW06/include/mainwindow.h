#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexworker.h"
#include <QDebug>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QTextCursor>
#include <QThread>

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
    void onFinishedIndexing(QString const &);
    void onStartedFinding();
    void onFinishedFinding(QString const &);

signals:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);
    void interrupt();

private:
    struct CursorPosition {
        qsizetype occurrenceIndex;
        SubstringOccurrence *document;
    } cursor;

    void renderText();

    Ui::MainWindow *ui;
    QString currentDir;
    std::vector<SubstringOccurrence> currentOccurrences;
    size_t wordSize;
    QThread thread;
    QTextCursor defaultCursor;
    IndexWorker worker;
    QString curFileName;
};

#endif // MAINWINDOW_H
