#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexworker.h"
#include <QDebug>
#include <QListWidget>
#include <QMainWindow>
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
    void getOccurrence(SubstringOccurrence);

private slots:
    void onStartedIndexing();
    void onFinishedIndexing();
    void onStartedFinding();
    void onFinishedFinding();

signals:
    void indexate(QString const &path);
    void findSubstring(QString const &substring);

private:
    Ui::MainWindow *ui;
    std::vector<SubstringOccurrence> currentOccurrences;
    size_t wordSize;
    QThread thread;
    IndexWorker worker;
};

#endif // MAINWINDOW_H
