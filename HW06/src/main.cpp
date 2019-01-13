#include <QApplication>

#include "include/mainwindow.h"

#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    MainWindow mainWin;
    qSetMessagePattern("[%{if-debug}DEBUG%{endif}%{if-info}INFO%{endif}%{if-"
                       "warning}WARNING%{endif}%{if-"
                       "critical}CRITICAL%{endif}%{if-"
                       "fatal}FATAL%{endif}"
                       "]%{if-debug} "
                       "(%{file}:%{line})%{endif} %{message}");
    mainWin.show();
    return app.exec();
}
