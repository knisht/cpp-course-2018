#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "controlpanel.h"
#include <QMainWindow>
#include <QTreeView>
#include <QtWidgets>

class Overloader : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QMap<QString, int> indices;
    QString root;

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
signals:
    void focused(QModelIndex const &) const;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void click_to_add();
    void click_to_remove();
    void click_to_find_equal_for_one();
    void click_to_go_upper();
    void click_to_go_deeper();
    void emphasize(QModelIndex const &);

private:
    QTreeView directoryContents;
    QFileSystemModel *model;
    ControlPanel panel;
    QLayout *layout;
    Overloader *delegate;
    QVector<QVector<QString>> indices;
    QString root;
    std::optional<QModelIndex> emphasedIndex;
};

#endif
