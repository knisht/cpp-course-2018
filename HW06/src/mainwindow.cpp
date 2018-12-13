#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), cursor({-1, nullptr}), ui(new Ui::MainWindow),
      currentDir("."), wordSize(0), thread(new QThread()), curFileName("")
{
    ui->setupUi(this);
    ui->label->setText("Current dir: " + QDir(".").canonicalPath());
    worker.moveToThread(&thread);
    connect(this, SIGNAL(indexate(QString const &)), &worker,
            SLOT(indexate(QString const &)));
    connect(this, SIGNAL(destroyed()), &thread, SLOT(quit()));
    connect(&worker, SIGNAL(startedIndexing()), this,
            SLOT(onStartedIndexing()));
    connect(&worker, SIGNAL(finishedIndexing(QString const &)), this,
            SLOT(onFinishedIndexing(QString const &)));
    connect(&worker, SIGNAL(startedFinding()), this, SLOT(onStartedFinding()));
    connect(&worker, SIGNAL(finishedFinding(QString const &)), this,
            SLOT(onFinishedFinding(QString const &)));
    connect(this, SIGNAL(findSubstring(QString const &)), &worker,
            SLOT(findSubstring(QString const &)));
    connect(&worker, SIGNAL(occurrenceFound(SubstringOccurrence const &)), this,
            SLOT(getOccurrence(SubstringOccurrence const &)));
    connect(&worker, SIGNAL(determinedFilesAmount(qint64)), this,
            SLOT(setProgressBarLimit(qint64)));
    connect(&worker, SIGNAL(progressChanged(qint64)), this,
            SLOT(changeProgressBarValue(qint64)));
    thread.start();
    defaultCursor = ui->filesContent->textCursor();
    ui->filesContent->setCursorWidth(0);
    ui->filesContent->setAcceptRichText(false);
    emit indexate(".");
    qDebug() << "thread start";
}

void MainWindow::findSubstring()
{
    stopActions();
    QString content = ui->stringInput->toPlainText();
    qDebug() << content;
    if (content.size() == 0) {
        return;
    }
    currentOccurrences.clear();
    ui->filesWidget->clear();
    emit findSubstring(content);
    wordSize = static_cast<size_t>(content.size());
    // TODO: no files message
}

void MainWindow::renderText()
{
    if (curFileName == "") {
        return;
    }
    QFile file(curFileName);
    file.open(QFile::ReadOnly);
    ui->filesContent->clear();
    ui->filesContent->setTextCursor(defaultCursor);
    //    ui->filesContent->setText(file.readAll());
    ui->filesContent->document()->setPlainText(file.readAll());
    file.close();
    QTextCharFormat fmt;
    fmt.setBackground(QColor{200, 100, 100, 255});
    ui->filesContent->textCursor().clearSelection();
    for (auto &&it : currentOccurrences) {
        // TODO: make hashmap
        if (it.filename == curFileName) {
            for (size_t position : it.occurrences) {
                QTextCursor cursor(ui->filesContent->textCursor());
                cursor.setPosition(static_cast<int>(position),
                                   QTextCursor::MoveAnchor);
                cursor.setPosition(static_cast<int>(position + wordSize),
                                   QTextCursor::KeepAnchor);
                cursor.setCharFormat(fmt);
                ui->filesContent->setTextCursor(cursor);
            }
            cursor = {-1, &it};
            break;
        }
    }
}

void MainWindow::getFileContent(QListWidgetItem *item)
{
    qDebug() << item->text();
    curFileName = item->text();
    ui->currentFileLabel->setText("Opened file: " +
                                  QFileInfo(curFileName).fileName());
    renderText();
}

void MainWindow::changeDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Open directory", currentDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    qInfo() << "Directory chosen:" << dir;
    if (dir != "") {
        emit indexate(dir);
        currentDir = dir;
        // TODO: relative path (with root processing)
        ui->label->setText("Current dir: " + currentDir);
    }
}

void MainWindow::nextOccurrence()
{
    if (cursor.document == nullptr) {
        return;
    }
    if (cursor.occurrenceIndex == -1) {
        cursor.occurrenceIndex = 0;
    } else {
        cursor.occurrenceIndex =
            (cursor.occurrenceIndex + 1) % cursor.document->occurrences.size();
    }
    QTextCursor textCursor(ui->filesContent->textCursor());
    textCursor.setPosition(
        static_cast<int>(cursor.document->occurrences[cursor.occurrenceIndex]),
        QTextCursor::MoveAnchor);
    textCursor.setPosition(
        static_cast<int>(cursor.document->occurrences[cursor.occurrenceIndex] +
                         wordSize),
        QTextCursor::KeepAnchor);
    ui->filesContent->setTextCursor(textCursor);
}

void MainWindow::previousOccurrence()
{

    if (cursor.document == nullptr) {
        return;
    }
    if (cursor.occurrenceIndex == -1) {
        cursor.occurrenceIndex = 0;
    } else {
        cursor.occurrenceIndex =
            (cursor.occurrenceIndex + cursor.document->occurrences.size() - 1) %
            cursor.document->occurrences.size();
    }
    QTextCursor textCursor(ui->filesContent->document());
    textCursor.setPosition(
        static_cast<int>(cursor.document->occurrences[cursor.occurrenceIndex]),
        QTextCursor::MoveAnchor);
    textCursor.setPosition(
        static_cast<int>(cursor.document->occurrences[cursor.occurrenceIndex] +
                         wordSize),
        QTextCursor::KeepAnchor);
    ui->filesContent->setTextCursor(textCursor);
}

MainWindow::~MainWindow()
{
    stopActions();
    thread.quit();
    thread.exit();
    delete ui;
}

void MainWindow::onStartedIndexing()
{
    ui->statusbar->showMessage("Indexing...");
}

void MainWindow::onFinishedIndexing(QString const &result)
{
    if (result == "") {
        ui->statusbar->showMessage("Indexing finished successfully");
    } else {
        ui->statusbar->showMessage("Indexing failed: " + result);
    }
}

void MainWindow::onStartedFinding()
{
    ui->statusbar->showMessage("Finding occurrences...");
}

void MainWindow::onFinishedFinding(QString const &result)
{
    if (result == "") {
        ui->statusbar->showMessage("Finding finished successfully");
        ui->progressBar->setValue(ui->progressBar->maximum());
    } else {
        ui->statusbar->showMessage("Finding failed: " + result);
    }
}

void MainWindow::getOccurrence(SubstringOccurrence const &oc)
{
    //    if (currentOccurrences.size() % 100 == 0) {
    //        ui->progressBar->setValue(ui->progressBar->value() + 100);
    //    }
    //    if (currentOccurrences.size() == ui->progressBar->maximum()) {
    //        ui->progressBar->setValue(ui->progressBar->maximum());
    //    }
    //    NOTE: better to keep above lines commented, otherwise program
    //    performance is much slower
    currentOccurrences.push_back(oc);
    ui->filesWidget->addItem(oc.filename);
    if (oc.filename == curFileName) {
        if (QFileInfo(curFileName).size() < 3000000) {
            renderText();
        }
    }
}

void MainWindow::setProgressBarLimit(qint64 limit)
{
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(limit);
    ui->progressBar->setValue(0);
}

void MainWindow::changeProgressBarValue(qint64 delta)
{
    ui->progressBar->setValue(ui->progressBar->value() + delta);
}

void MainWindow::stopActions()
{
    qInfo() << "Cancelling thread";
    worker.interrupt();
    thread.start();
}
