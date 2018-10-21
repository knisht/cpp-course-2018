#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "controlpanel.h"
#include <QMainWindow>
#include <QTreeView>
#include <QtWidgets>

class Overloader : public QStyledItemDelegate
{
public:
    QMap<QString, int> indices;
    QString root;

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QString const &root);

    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void click();

private:
    QTreeView directoryContents;
    QFileSystemModel *model;
    ControlPanel panel;
    QLayout *layout;
    Overloader *delegate;
    QVector<QVector<QString>> indices;
    QString root;
};

#endif
