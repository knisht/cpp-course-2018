#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "include/TrigramIndex.h"
#include <QListWidget>
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void findSubstring();
    void getFileContent(QListWidgetItem *);

private:
    Ui::MainWindow *ui;
    TrigramIndex index;
    std::optional<std::vector<TrigramIndex::SubstringOccurrence>>
        currentOccurrences;
    size_t wordSize;
};

#endif // MAINWINDOW_H
