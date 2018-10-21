#include "../include/mainwindow.h"
#include "../include/checker.hpp"
#include "../include/controlpanel.h"
#include <QFileSystemModel>
#include <QRect>
#include <QStyle>
#include <iostream>
#include <string>
#include <vector>

// Functional object to biject equal numbers to equal RGB colors
struct ColorGenerator {
private:
    QMap<int, QColor> memoized;
    QRandomGenerator generator;

public:
    ColorGenerator() : memoized(), generator() {}

    QColor operator()(int index)
    {
        if (!memoized.contains(index)) {
            memoized[index] =
                QColor{static_cast<int>(generator.generate() % 256),
                       static_cast<int>(generator.generate() % 256),
                       static_cast<int>(generator.generate() % 256), 100};
        }
        return memoized[index];
    }
} static colorGenerator;

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
    if (option.state & QStyle::State_Selected) {
        QBrush brush;
        QColor color;
        color.setRgb(0, 153, 255, 150);
        painter->fillRect(option.rect, color);
        focused(index);
    }

    if (option.state & QStyle::State_MouseOver) {
        QColor color;
        color.setRgb(102, 204, 255, 50);
        painter->fillRect(option.rect, color);
    }
    QModelIndex const &primaryFilename = index.siblingAtColumn(0);
    if (indices.contains(getFileName(primaryFilename))) {
        QRect internalRect = option.rect;
        painter->fillRect(
            internalRect,
            colorGenerator(indices[getFileName(primaryFilename)]));
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
    opt.setAlignment(Qt::AlignBottom);
    painter->drawText(rect, target, opt);
}

MainWindow::MainWindow()
    : model(new QFileSystemModel), layout(new QHBoxLayout),
      delegate(new Overloader), root(QDir::currentPath()), emphasedIndex()
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
    resize(availableSize * 2 / 3);
    directoryContents.setColumnWidth(0, directoryContents.width() / 3);

    directoryContents.setWindowTitle(QObject::tr("Dir View"));
    layout->addWidget(&directoryContents);
    layout->addWidget(&panel);
    directoryContents.setItemDelegate(delegate);
    delegate->root = root;
    setLayout(layout);
    connect(&panel.get_add_button(), SIGNAL(released()), this,
            SLOT(click_to_add()));
    connect(&panel.get_remove_button(), SIGNAL(released()), this,
            SLOT(click_to_remove()));
    connect(delegate, SIGNAL(focused(QModelIndex const &)), this,
            SLOT(emphasize(QModelIndex const &)));
    connect(&panel.get_search_for_one_button(), SIGNAL(released()), this,
            SLOT(click_to_find_equal_for_one()));
    connect(&panel.go_to_parent_button(), SIGNAL(released()), this,
            SLOT(click_to_go_upper()));
    connect(&panel.go_to_inner_dir_button(), SIGNAL(released()), this,
            SLOT(click_to_go_deeper()));
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete model;
    delete layout;
    delete delegate;
}

void MainWindow::click_to_add()
{
    std::vector<std::vector<std::string>> data =
        core::group_all(QDir::currentPath().toStdString());
    delegate->indices.clear();
    int index_count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i].size() == 1) {
            continue;
        }
        auto &it = data[i];
        for (auto jt : it) {
            delegate->indices[QString::fromStdString(jt)] = index_count;
        }
        ++index_count;
    }
    repaint();
}

void MainWindow::click_to_remove()
{
    if (!emphasedIndex) {
        qDebug() << "Some file must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {
        model->remove(emphasedIndex.value());
    } else {
        qDebug() << "Failed to delete file" << endl;
    }
    repaint();
}

void MainWindow::click_to_find_equal_for_one()
{
    if (!emphasedIndex) {
        qDebug() << "Some file be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {
        std::vector<std::string> data =
            core::group_for(fileInfo.filePath().toStdString());
        delegate->indices.clear();
        for (auto &it : data) {
            delegate->indices[QString::fromStdString(it)] = 0;
        }
    } else {
        qDebug() << "Failed to access the file" << endl;
    }
    repaint();
}

void MainWindow::click_to_go_upper()
{
    if (root == "" || root == "/") {
        qDebug() << "Attempt to go upper than root" << endl;
        return;
    }
    directoryContents.setRootIndex(directoryContents.rootIndex().parent());
    root = getFileName(directoryContents.rootIndex());
}

void MainWindow::click_to_go_deeper()
{
    if (!emphasedIndex) {
        qDebug() << "Some directory must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isDir()) {
        directoryContents.setRootIndex(
            emphasedIndex.value().siblingAtColumn(0));
        root = getFileName(emphasedIndex.value().siblingAtColumn(0));
    } else {
        qDebug() << "Selected object is not a directory" << endl;
    }
}

void MainWindow::emphasize(QModelIndex const &a) { emphasedIndex = a; }
