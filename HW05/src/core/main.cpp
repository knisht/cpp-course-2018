#include "include/gui/mainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    gui::MainWindow window = gui::MainWindow();
    window.show();
    return app.exec();
}
