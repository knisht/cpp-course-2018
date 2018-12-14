#include <QApplication>

#include "include/mainwindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    MainWindow mainWin;
    qSetMessagePattern("[%{type}] %{appname}"
                       "%{if-debug} (%{file}:%{line})%{endif} - %{message}");
    mainWin.show();
    return app.exec();
}
