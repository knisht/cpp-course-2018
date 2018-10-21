#ifndef DIRECTORY_VIEW_H
#define DIRECTORY_VIEW_H

#include <QtWidgets>
namespace gui
{

class DirectoryTreeStyleDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    QMap<QString, int> coloredIndices;
    QFileSystemModel model;

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    void flushColored();
    void store(QString const &filename, int group);
signals:
    void focusOn(QModelIndex const &) const;
};

class DirectoryView : public QWidget
{
    Q_OBJECT
public:
    DirectoryView();
    ~DirectoryView();
    QSize sizeHint() const;
    void resize(const QSize &);
    bool event(QEvent *ev);
public slots:
    void findDuplicatesForEveryone();
    void removeFile();
    void findDuplicatesForParticular();
    void changeDirUp();
    void changeDirDown();

private slots:
    void emphasizeIndex(QModelIndex const &);

private:
    QTreeView directoryContents;
    QFileSystemModel model;
    DirectoryTreeStyleDelegate delegate;
    QVector<QVector<QString>> indices;
    QString root;
    std::optional<QModelIndex> emphasedIndex;
};
} // namespace gui
#endif
