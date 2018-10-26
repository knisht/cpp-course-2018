#include "include/gui/mainwindow.h"
#include "include/core/checker.hpp"
#include "include/gui/controlpanel.h"
#include <QFileSystemModel>
#include <QRect>
#include <QStyle>
#include <iostream>
#include <string>
#include <vector>

namespace gui
{

MainWindow::MainWindow() : layout(new QHBoxLayout)
{
    const QSize availableSize =
        QApplication::desktop()->availableGeometry(this).size() * 2 / 3;
    resize(availableSize);
    setWindowTitle("File checker");
    layout->addWidget(&view);
    layout->addWidget(&panel);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    setLayout(layout.get());
    configureActions();
}

MainWindow::~MainWindow() {}

void MainWindow::configureActions()
{

    connect(&panel, SIGNAL(splitEverything()), &view,
            SLOT(findDuplicatesForEveryone()));
    connect(&panel, SIGNAL(removeFiles()), &view, SLOT(removeFile()));
    connect(&panel, SIGNAL(splitForOne()), &view,
            SLOT(findDuplicatesForParticular()));
    connect(&panel, SIGNAL(goUpper()), &view, SLOT(changeDirUp()));
    connect(&panel, SIGNAL(goDeeper()), &view, SLOT(changeDirDown()));
    connect(&panel, SIGNAL(collapseAll()), &view, SLOT(collapseEverything()));
}

} // namespace gui
