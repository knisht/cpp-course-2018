#include "mainwindow.h"
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

    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    MainWindow window = MainWindow(QDir::currentPath());
    window.show();
    return app.exec();
}
