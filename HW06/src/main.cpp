#include <QApplication>

#include "include/mainwindow.h"

#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    MainWindow mainWin;
    qSetMessagePattern("[%{type}] %{appname} %{if-debug} "
                       "(%{file}:%{line})%{endif} - %{message}");
    mainWin.show();
    return app.exec();
}
