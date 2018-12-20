#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexdriver.h"
#include "textviewdriver.h"
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
    void getOccurrence(QString const &);
    void setProgressBarLimit(qint64 limit);
    void changeProgressBarValue(qint64 delta);
    void stopActions();
    void openFileManager();
    void openEditor();

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
    void renderText();
    void highlightSpecificOccurrence();
    void rtxt(QTextCharFormat);

    qsizetype occurrenceIndex;
    IndexDriver worker;
    std::unique_ptr<Ui::MainWindow> ui;
    QString currentDir;
    std::vector<size_t> currentWordPositionsInFile;
    QTextCursor defaultCursor;
    TextViewDriver textDriver;
    QString currentFileName;
    QString currentWord;
};

#endif // MAINWINDOW_H
