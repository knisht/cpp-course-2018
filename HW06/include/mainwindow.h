#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexdriver.h"
#include "settingswindow.h"
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
    void getOccurrence(SubstringOccurrence const &);
    void setProgressBarLimit(qint64 limit);
    void changeProgressBarValue(qint64 delta);
    void stopActions();
    void openFileManager();
    void openEditor();
    void receiveSettings(bool asyncSearch, bool liveColoring,
                         bool fileWatching);
    void indexate(QString const &path);

private slots:
    void onStartedIndexing();
    void onFinishedIndexing(QString const &);
    void onStartedFinding();
    void onFinishedFinding(QString const &);
    void showSettings();

signals:
    void findSubstring(QString const &substring);
    void interrupt();

private:
    void highlightSpecificOccurrence();
    void rtxt(QTextCharFormat);

    IndexDriver worker;
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<SettingsWindow> settingsWindow;
    QString currentDir;
    QString currentWord;
    QSettings settings;
};

#endif // MAINWINDOW_H
