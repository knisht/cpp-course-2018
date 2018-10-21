#include "mainwindow.h"
#include "../include/checker.hpp"
#include "controlpanel.h"
#include <QFileSystemModel>
#include <QRect>
#include <QStyle>
#include <iostream>
#include <string>
#include <vector>

static const QVector<QColor> colors{
    QColor(200, 0, 0, 127),     QColor(0, 200, 0, 127),
    QColor(0, 0, 200, 127),     QColor(200, 0, 200, 127),
    QColor(0, 200, 200, 127),   QColor(200, 200, 0, 127),
    QColor(100, 100, 100, 127), QColor(150, 124, 0, 127),
    QColor(124, 150, 0, 127),   QColor(0, 124, 200, 127)};

QString getFileName(QModelIndex const &index)
{
    if (index.data().toString() == "/") {
        return "";
    } else {
        return getFileName(index.parent()) + "/" + index.data().toString();
    }
}

void Overloader::paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
    if (option.state & QStyle::State_HasFocus) {
        QBrush brush;
        QColor color;
        color.setRgb(0, 153, 255, 150);
        // FIX IT
        QRect internalRect = option.rect;
        internalRect.setX(0);
        internalRect.setWidth(1024);
        painter->fillRect(internalRect, color);
    }

    if (option.state & QStyle::State_MouseOver) {
        QBrush brush;
        QColor color;
        color.setRgb(102, 204, 255, 50);
        painter->fillRect(option.rect, color);
    }
    if (index.column() == 0) {
        if (indices.contains(getFileName(index))) {
            QRect internalRect = option.rect;
            internalRect.setX(0);
            internalRect.setWidth(1024);
            painter->fillRect(internalRect,
                              colors[indices[getFileName(index)]]);
        }
    }
    QFileSystemModel model;
    model.fileIcon(index).paint(painter, option.rect, Qt::AlignLeft);
    QString target = index.data().toString();
    QRect rect = option.rect;
    if (index.column() == 0) {
        rect.setX(rect.x() +
                  model.fileIcon(index).availableSizes()[0].rwidth() * 2);
    }
    QTextOption opt;
    opt.setAlignment(Qt::AlignVCenter);
    painter->drawText(rect, target, opt);
}

MainWindow::MainWindow(QString const &root)
    : model(new QFileSystemModel), layout(new QHBoxLayout),
      delegate(new Overloader), root(root)
{
    model->setRootPath("");
    directoryContents.setModel(model);
    const QModelIndex rootIndex = model->index(QDir::cleanPath(root));
    directoryContents.setRootIndex(rootIndex);
    directoryContents.setAnimated(true);
    directoryContents.setIndentation(20);
    directoryContents.setSortingEnabled(true);
    const QSize availableSize =
        QApplication::desktop()->availableGeometry(&directoryContents).size();
    resize(availableSize / 2);
    directoryContents.setColumnWidth(0, directoryContents.width() / 3);

    directoryContents.setWindowTitle(QObject::tr("Dir View"));
    layout->addWidget(&directoryContents);
    layout->addWidget(&panel);
    directoryContents.setItemDelegate(delegate);
    delegate->root = root;
    setLayout(layout);
    connect(&panel.get_button(), SIGNAL(released()), this, SLOT(click()));
}

MainWindow::~MainWindow() { qDebug("destructed"); }

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete model;
    delete layout;
    delete delegate;
}

void MainWindow::click()
{
    std::cout << QDir::currentPath().toStdString() << std::endl;
    std::vector<std::vector<std::string>> data =
        core::group_all(QDir::currentPath().toStdString());
    delegate->indices.clear();
    int index_count = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (data[i].size() == 1) {
            continue;
        }
        auto &it = data[i];
        for (auto jt : it) {
            delegate->indices[QString::fromStdString(jt)] = index_count;
        }
        ++index_count;
    }
}
