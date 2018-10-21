#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "controlpanel.h"
#include "directoryview.h"
#include <QMainWindow>
#include <QTreeView>
#include <QtWidgets>

namespace gui
{

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

    ~MainWindow() override;

private:
    void configureActions();

    QLayout *layout;
    ControlPanel panel;
    DirectoryView view;
};
} // namespace gui

#endif
