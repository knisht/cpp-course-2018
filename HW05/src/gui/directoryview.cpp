#include "include/gui/directoryview.h"
#include "include/core/checker.hpp"
#include <QtWidgets>

namespace gui
{

// Functional object to biject equal numbers to equal RGB colors
namespace
{
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
} // namespace
DirectoryTreeStyleDelegate::DirectoryTreeStyleDelegate(
    QFileSystemModel const *model)
{
    this->model = model;
}

void DirectoryTreeStyleDelegate::paint(QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        QColor color;
        color.setRgb(0, 153, 255, 150);
        painter->fillRect(option.rect, color);
        emit focusOn(index);
    }
    if (option.state & QStyle::State_MouseOver) {
        QColor color;
        color.setRgb(102, 204, 255, 50);
        painter->fillRect(option.rect, color);
    }
    QModelIndex const &primaryFilename = index.siblingAtColumn(0);
    if (coloredIndices.contains(model->filePath(primaryFilename))) {
        painter->fillRect(
            option.rect,
            colorGenerator(coloredIndices[model->filePath(primaryFilename)]));
    }
    model->fileIcon(index).paint(painter, option.rect, Qt::AlignLeft);
    QRect rect = option.rect;
    QString targetText = index.data().toString();
    QFont font = painter->font();
    if (index.column() == 0) {
        QFileInfo info(model->filePath(primaryFilename));
        if (info.isSymLink()) {
            QFont font = painter->font();
            font.setItalic(true);
            painter->setFont(font);
            targetText +=
                " → " + info.dir().relativeFilePath(info.symLinkTarget());
        }
        rect.setX(rect.x() +
                  model->fileIcon(index).availableSizes()[0].rwidth() * 2);
    }
    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapMode::NoWrap);
    opt.setAlignment(Qt::AlignVCenter);
    painter->drawText(rect, targetText, opt);
    painter->setFont(font);
}

void DirectoryTreeStyleDelegate::flushColored() { coloredIndices.clear(); }

void DirectoryTreeStyleDelegate::store(QString const &filename, int group)
{
    coloredIndices[filename] = group;
}

DirectoryView::DirectoryView() : model(), delegate(&model), emphasizedIndex()
{
    model.setRootPath("");
    model.setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    directoryContents.setModel(&model);
    const QModelIndex rootIndex =
        model.index(QDir::cleanPath(QDir::currentPath()));
    directoryContents.setRootIndex(rootIndex);
    this->setSizePolicy(directoryContents.sizePolicy());
    directoryContents.resize(this->size());
    directoryContents.setColumnWidth(0, directoryContents.width() / 2);
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
        directoryContents.setColumnWidth(0, directoryContents.width() / 2);
    }
    return QWidget::event(ev);
}

DirectoryView::~DirectoryView() = default;

void DirectoryView::findDuplicatesForEveryone()
{
    std::vector<std::vector<std::string>> data = core::group_all(
        model.filePath(directoryContents.rootIndex()).toStdString());
    delegate.flushColored();
    int index_count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i].size() == 1) {
            continue;
        }
        auto &it = data[i];
        for (auto &jt : it) {
            delegate.store(QString::fromStdString(jt), index_count);
            deepExpand(
                directoryContents.rootIndex(),
                model.index(QDir::cleanPath(QString::fromStdString(jt))));
        }
        ++index_count;
    }
    repaint();
}

void DirectoryView::removeFile()
{
    if (!emphasizedIndex) {
        qDebug() << "[INFO] Some file must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(
        model.filePath(emphasizedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {
        model.remove(emphasizedIndex.value());
    } else {
        qDebug() << "[ERROR] Failed to delete file" << endl;
    }
    repaint();
}

void DirectoryView::deepExpand(QModelIndex const &limit,
                               QModelIndex const &current)
{
    if (limit == current || model.filePath(current) == "/") {
        return;
    }
    directoryContents.expand(current);
    deepExpand(limit, current.parent());
}

void DirectoryView::findDuplicatesForParticular()
{
    if (!emphasizedIndex) {
        qDebug() << "[INFO] Some file must be selected" << endl;
        return;
    }
    QFileInfo fileInfo(
        model.filePath(emphasizedIndex.value().siblingAtColumn(0)));
    if (fileInfo.exists() && fileInfo.isFile()) {

        std::vector<std::string> data = core::group_for(
            fileInfo.canonicalFilePath().toStdString(),
            model.filePath(directoryContents.rootIndex()).toStdString());
        delegate.flushColored();
        for (auto &it : data) {
            delegate.store(QString::fromStdString(it), 0);
            deepExpand(
                directoryContents.rootIndex(),
                model.index(QDir::cleanPath(QString::fromStdString(it))));
        }
    } else {
        qDebug() << "[ERROR] Failed to access the file" << endl;
    }
    repaint();
}

void DirectoryView::changeDirUp()
{
    QString root = model.filePath(directoryContents.rootIndex());
    if (root == "" || root == "/") {
        qDebug() << "[ERROR] Attempt to go upper than root" << endl;
        return;
    }
    directoryContents.setRootIndex(directoryContents.rootIndex().parent());
}

void DirectoryView::changeDirDown()
{
    if (!emphasizedIndex) {
        qDebug() << "[INFO] Some directory must be selected" << endl;
        return;
    }
    QString path = model.filePath(emphasizedIndex.value().siblingAtColumn(0));
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && fileInfo.isDir()) {
        directoryContents.setRootIndex(model.index(path));
    } else {
        qDebug() << "[INFO] Selected object is not a directory" << endl;
    }
}

void DirectoryView::collapseEverything()
{
    directoryContents.collapseAll();
    if (emphasizedIndex) {
        deepExpand(directoryContents.rootIndex(), emphasizedIndex.value());
    }
}

void DirectoryView::emphasizeIndex(QModelIndex const &a)
{
    emphasizedIndex = a;
}

void DirectoryView::changeDirOnClick(QModelIndex const &a)
{
    QFileInfo fileInfo(model.filePath(a));
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
