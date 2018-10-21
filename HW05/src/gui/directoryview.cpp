#include "include/gui/directoryview.h"
#include "include/core/checker.hpp"
#include <QtWidgets>

namespace gui
{

// Functional object to biject equal numbers to equal RGB colors
struct ColorGenerator {
private:
    QMap<int, QColor> memoizedColors;
    QRandomGenerator generator;

public:
    ColorGenerator() : memoizedColors(), generator() {}

    QColor operator()(int index)
    {
        if (!memoizedColors.contains(index)) {
            memoizedColors[index] =
                QColor{static_cast<int>(generator.generate() % 256),
                       static_cast<int>(generator.generate() % 256),
                       static_cast<int>(generator.generate() % 256), 100};
        }
        return memoizedColors[index];
    }
} static colorGenerator;

static QString getFileName(QModelIndex const &index, int is_directory = false)
{
    if (index.data().toString() == "/") {
        return "/";
    } else {
        return getFileName(index.parent(), true) + index.data().toString() +
               (is_directory ? "/" : "");
    }
}

void DirectoryTreeStyleDelegate::paint(QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        QColor color;
        color.setRgb(0, 153, 255, 150);
        painter->fillRect(option.rect, color);
        emit focusOn(index); // correct naming !!!
    }
    if (option.state & QStyle::State_MouseOver) {
        QColor color;
        color.setRgb(102, 204, 255, 50);
        painter->fillRect(option.rect, color);
    }
    QModelIndex const &primaryFilename = index.siblingAtColumn(0);
    if (coloredIndices.contains(getFileName(primaryFilename))) {
        painter->fillRect(
            option.rect,
            colorGenerator(coloredIndices[getFileName(primaryFilename)]));
    }
    model.fileIcon(index).paint(painter, option.rect, Qt::AlignLeft);
    QRect rect = option.rect;
    if (index.column() == 0) {
        rect.setX(rect.x() +
                  model.fileIcon(index).availableSizes()[0].rwidth() * 2);
    }
    QTextOption opt;
    opt.setAlignment(Qt::AlignBottom);
    QString targetText = index.data().toString();
    painter->drawText(rect, targetText, opt);
}

void DirectoryTreeStyleDelegate::flushColored() { coloredIndices.clear(); }

void DirectoryTreeStyleDelegate::store(QString const &filename, int group)
{
    coloredIndices[filename] = group;
}

DirectoryView::DirectoryView() : model(), delegate(), emphasizedIndex()
{
    model.setRootPath("");
    directoryContents.setModel(&model);
    const QModelIndex rootIndex =
        model.index(QDir::cleanPath(QDir::currentPath()));
    directoryContents.setRootIndex(rootIndex);
    this->setSizePolicy(directoryContents.sizePolicy());
    directoryContents.resize(this->size());
    directoryContents.setColumnWidth(0, directoryContents.width() / 3);
    directoryContents.setItemDelegate(&delegate);
    connect(&delegate, SIGNAL(focusOn(QModelIndex const &)), this,
            SLOT(emphasizeIndex(QModelIndex const &)));
    connect(&directoryContents, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(changeDirOnClick(const QModelIndex &)));
    directoryContents.setParent(this);
}

bool DirectoryView::event(QEvent *ev)
{
    if (ev->type() == QEvent::Resize) {
        directoryContents.resize(this->size());
        directoryContents.setColumnWidth(0, directoryContents.width() / 3);
    }
    return QWidget::event(ev);
}

DirectoryView::~DirectoryView() {}

void DirectoryView::findDuplicatesForEveryone()
{
    std::vector<std::vector<std::string>> data = core::group_all(
        getFileName(directoryContents.rootIndex()).toStdString());
    delegate.flushColored();
    int index_count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i].size() == 1) {
            continue;
        }
        auto &it = data[i];
        for (auto jt : it) {
            delegate.store(QString::fromStdString(jt), index_count);
        }
        ++index_count;
    }
    repaint();
}

void DirectoryView::removeFile()
{
    if (!emphasizedIndex) {
        qDebug() << "Some file must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasizedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {
        model.remove(emphasizedIndex.value());
    } else {
        qDebug() << "Failed to delete file" << endl;
    }
    repaint();
}

void DirectoryView::massiveExpand(QModelIndex const &limit,
                                  QModelIndex const &current)
{
    if (limit == current) {
        return;
    }
    directoryContents.expand(current);
    massiveExpand(limit, current.parent());
}

void DirectoryView::findDuplicatesForParticular()
{
    if (!emphasizedIndex) {
        qDebug() << "Some file must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasizedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {
        std::vector<std::string> data = core::group_for(
            fileInfo.filePath().toStdString(),
            getFileName(directoryContents.rootIndex()).toStdString());
        delegate.flushColored();
        for (auto &it : data) {
            delegate.store(QString::fromStdString(it), 0);
            massiveExpand(
                directoryContents.rootIndex(),
                model.index(QDir::cleanPath(QString::fromStdString(it))));
        }
    } else {
        qDebug() << "Failed to access the file" << endl;
    }
    repaint();
}

void DirectoryView::changeDirUp()
{
    QString root = getFileName(directoryContents.rootIndex());
    if (root == "" || root == "/") {
        qDebug() << "Attempt to go upper than root" << endl;
        return;
    }
    directoryContents.setRootIndex(directoryContents.rootIndex().parent());
}

void DirectoryView::changeDirDown()
{
    if (!emphasizedIndex) {
        qDebug() << "Some directory must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(getFileName(emphasizedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isDir()) {
        directoryContents.setRootIndex(
            emphasizedIndex.value().siblingAtColumn(0));
    } else {
        qDebug() << "Selected object is not a directory" << endl;
    }
}

void DirectoryView::emphasizeIndex(QModelIndex const &a)
{
    emphasizedIndex = a;
}

void DirectoryView::changeDirOnClick(QModelIndex const &a)
{
    QFileInfo fileInfo(getFileName(a));
    if (fileInfo.exists() && fileInfo.isDir()) {
        changeDirDown();
    }
}

void DirectoryView::resize(const QSize &size)
{
    directoryContents.resize(size);
}

QSize DirectoryView::sizeHint() const { return directoryContents.sizeHint(); }

} // namespace gui
