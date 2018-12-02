#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), index("."), wordSize(0)
{
    ui->setupUi(this);
    // TODO: default view
}

void MainWindow::findSubstring()
{
    QString content = ui->stringInput->toPlainText();
    qDebug() << content;
    if (content.size() == 0) {
        return;
    }
    currentOccurrences = index.findSubstring(content);
    wordSize = static_cast<size_t>(content.size());
    // TODO: no files message
    ui->filesWidget->clear();
    for (auto &&it : currentOccurrences.value()) {
        ui->filesWidget->addItem(it.filename);
        qDebug() << it.filename;
    }
}

void MainWindow::getFileContent(QListWidgetItem *item)
{
    qDebug() << item->text();
    QFile file(item->text());
    file.open(QFile::ReadOnly);
    int begin = 5;
    int end = 10;
    QTextCharFormat fmt;
    fmt.setBackground(QColor{50, 200, 200, 200});
    ui->filesContent->setText(file.readAll());
    for (auto &&it : currentOccurrences.value()) {
        // TODO: make hashmap
        if (it.filename == item->text()) {
            for (size_t position : it.occurrences) {
                QTextCursor cursor(ui->filesContent->document());
                cursor.setPosition(position, QTextCursor::MoveAnchor);
                cursor.setPosition(position + wordSize,
                                   QTextCursor::KeepAnchor);
                cursor.setCharFormat(fmt);
            }
            break;
        }
    }

    file.close();
}

MainWindow::~MainWindow() { delete ui; }
