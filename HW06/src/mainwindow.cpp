#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), wordSize(0),
      thread(new QThread())
{
    ui->setupUi(this);
    ui->statusbar->showMessage("indexing...");
    ui->label->setText("Current dir: " + QDir(".").canonicalPath());
    worker.moveToThread(&thread);
    connect(this, SIGNAL(indexate(QString const &)), &worker,
            SLOT(indexate(QString const &)));
    connect(&worker, SIGNAL(finishedIndexing()), &thread, SLOT(quit()));
    connect(&worker, SIGNAL(startedIndexing()), this,
            SLOT(onStartedIndexing()));
    connect(&worker, SIGNAL(finishedIndexing()), this,
            SLOT(onFinishedIndexing()));
    connect(&worker, SIGNAL(startedFinding()), this, SLOT(onStartedFinding()));
    connect(&worker, SIGNAL(finishedFinding()), this,
            SLOT(onFinishedFinding()));
    connect(this, SIGNAL(findSubstring(QString const &)), &worker,
            SLOT(findSubstring(QString const &)));
    connect(&worker, SIGNAL(occurrenceFound(SubstringOccurrence)), this,
            SLOT(getOccurrence(SubstringOccurrence)));
    thread.start();
    emit indexate("../../");
    qDebug() << "thread start";
}

void MainWindow::findSubstring()
{
    QString content = ui->stringInput->toPlainText();
    qDebug() << content;
    if (content.size() == 0) {
        return;
    }
    currentOccurrences.clear();
    thread.start();
    ui->filesWidget->clear();
    emit findSubstring(content);
    qDebug() << "here!";
    wordSize = static_cast<size_t>(content.size());
    // TODO: no files message
}

void MainWindow::getFileContent(QListWidgetItem *item)
{
    qDebug() << item->text();
    QFile file(item->text());
    file.open(QFile::ReadOnly);
    QTextCharFormat fmt;
    fmt.setBackground(QColor{50, 200, 200, 200});
    ui->filesContent->setText(file.readAll());
    for (auto &&it : currentOccurrences) {
        // TODO: make hashmap
        if (it.filename == item->text()) {
            for (size_t position : it.occurrences) {
                QTextCursor cursor(ui->filesContent->document());
                cursor.setPosition(static_cast<int>(position),
                                   QTextCursor::MoveAnchor);
                cursor.setPosition(static_cast<int>(position + wordSize),
                                   QTextCursor::KeepAnchor);
                cursor.setCharFormat(fmt);
            }
            break;
        }
    }

    file.close();
}

void MainWindow::changeDirectory() { qDebug() << "not implemented"; }

void MainWindow::nextOccurrence() { qDebug() << "not implemented"; }

void MainWindow::previousOccurrence() { qDebug() << "not implemented"; }

MainWindow::~MainWindow()
{
    delete ui;
    thread.quit();
}

void MainWindow::onStartedIndexing()
{
    ui->statusbar->showMessage("Indexing...");
}

void MainWindow::onFinishedIndexing()
{
    ui->statusbar->showMessage("Indexing finished");
}

void MainWindow::onStartedFinding()
{
    ui->statusbar->showMessage("Finding occurrences...");
}

void MainWindow::onFinishedFinding()
{
    ui->statusbar->showMessage("Finding finished");
}

void MainWindow::getOccurrence(SubstringOccurrence oc)
{
    qDebug() << oc.filename << "!";
    currentOccurrences.push_back(oc);
    ui->filesWidget->addItem(oc.filename);
}
