#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), occurrenceIndex(-1), ui(new Ui::MainWindow),
      currentDir("."), currentWordPositionsInFile(), currentFileName("")
{
    ui->setupUi(this);
    ui->label->setText("Current dir: " + QDir(".").canonicalPath());
    connect(this, SIGNAL(indexate(QString const &)), &worker,
            SLOT(indexate(QString const &)));
    connect(&worker, SIGNAL(startedIndexing()), this,
            SLOT(onStartedIndexing()));
    connect(&worker, SIGNAL(finishedIndexing(QString const &)), this,
            SLOT(onFinishedIndexing(QString const &)));
    connect(&worker, SIGNAL(startedFinding()), this, SLOT(onStartedFinding()));
    connect(&worker, SIGNAL(finishedFinding(QString const &)), this,
            SLOT(onFinishedFinding(QString const &)));
    connect(this, SIGNAL(findSubstring(QString const &)), &worker,
            SLOT(findSubstring(QString const &)));
    connect(&worker, SIGNAL(properFileFound(QString const &)), this,
            SLOT(getOccurrence(QString const &)));
    connect(&worker, SIGNAL(determinedFilesAmount(qint64)), this,
            SLOT(setProgressBarLimit(qint64)));
    connect(&worker, SIGNAL(progressChanged(qint64)), this,
            SLOT(changeProgressBarValue(qint64)));
    defaultCursor = ui->filesContent->textCursor();
    ui->filesContent->setCursorWidth(0);
    ui->filesContent->setAcceptRichText(false);
    emit indexate(".");
}

void MainWindow::findSubstring()
{
    currentWord = ui->stringInput->toPlainText();
    if (currentWord.size() == 0) {
        return;
    }
    emit findSubstring(currentWord);
    currentWordPositionsInFile.clear();
    ui->filesWidget->clear();
}

void MainWindow::renderText()
{
    if (currentFileName == "") {
        return;
    }
    QFile file(currentFileName);
    file.open(QFile::ReadOnly);
    ui->filesContent->clear();
    ui->filesContent->setTextCursor(defaultCursor);
    ui->filesContent->document()->setPlainText(file.readAll());
    file.close();
    QTextCharFormat fmt;
    fmt.setBackground(QColor{200, 100, 100, 255});
    ui->filesContent->textCursor().clearSelection();
    for (size_t position : currentWordPositionsInFile) {
        QTextCursor cursor(ui->filesContent->textCursor());
        cursor.setPosition(static_cast<int>(position), QTextCursor::MoveAnchor);
        cursor.setPosition(static_cast<int>(position) + currentWord.size(),
                           QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt);
        ui->filesContent->setTextCursor(cursor);
    }
    occurrenceIndex = 0;
}

void MainWindow::getFileContent(QListWidgetItem *item)
{
    qInfo() << "Opening" << item->text();
    currentFileName = QDir(currentDir).absoluteFilePath(item->text());
    ui->currentFileLabel->setText("Opened file: " +
                                  QFileInfo(currentFileName).fileName());
    currentWordPositionsInFile =
        worker.getFileStat(currentFileName, currentWord);
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
        currentDir = QDir(dir).absolutePath();
        ui->label->setText("Current dir: " + currentDir);
    }
}

void MainWindow::highlightSpecificOccurrence()
{
    QTextCursor textCursor(ui->filesContent->textCursor());
    int position = static_cast<int>(
        currentWordPositionsInFile[static_cast<size_t>(occurrenceIndex)]);
    textCursor.setPosition(position, QTextCursor::MoveAnchor);
    textCursor.setPosition(position + currentWord.size(),
                           QTextCursor::KeepAnchor);
    ui->filesContent->setTextCursor(textCursor);
}

void MainWindow::nextOccurrence()
{
    if (occurrenceIndex == -1) {
        return;
    }

    occurrenceIndex =
        static_cast<qsizetype>(static_cast<size_t>(occurrenceIndex + 1) %
                               currentWordPositionsInFile.size());
    highlightSpecificOccurrence();
}

void MainWindow::previousOccurrence()
{
    if (occurrenceIndex == -1) {
        return;
    }

    occurrenceIndex = static_cast<qsizetype>(
        static_cast<size_t>(static_cast<size_t>(occurrenceIndex) +
                            currentWordPositionsInFile.size() - 1) %
        currentWordPositionsInFile.size());
    highlightSpecificOccurrence();
}

MainWindow::~MainWindow() { worker.interrupt(); }

void MainWindow::onStartedIndexing()
{
    ui->progressBar->reset();
    ui->statusbar->showMessage("Indexing...");
}

void MainWindow::onFinishedIndexing(QString const &result)
{
    if (result == "") {
        ui->statusbar->showMessage("Indexing finished successfully");
        ui->progressBar->setValue(ui->progressBar->maximum());
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

void MainWindow::getOccurrence(QString const &newFile)
{
    //    ui->progressBar->setValue(ui->progressBar->value() + 500);
    //    NOTE: better to keep above lines commented, otherwise program
    //    performance is much slower
    ui->filesWidget->addItem(QDir(currentDir).relativeFilePath(newFile));
    if (newFile == currentFileName) {
        if (QFileInfo(currentFileName).size() < 3000000) {
            currentWordPositionsInFile =
                worker.getFileStat(currentFileName, currentWord);
            renderText();
        } else {
            ui->filesContent->setText(
                "Sorry, but your file is too big to display it");
        }
    }
}

void MainWindow::setProgressBarLimit(qint64 limit)
{
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(static_cast<int>(limit));
    ui->progressBar->setValue(0);
}

void MainWindow::changeProgressBarValue(qint64 delta)
{
    ui->progressBar->setValue(
        static_cast<int>(ui->progressBar->value() + delta));
}

void MainWindow::openFileManager()
{
    if (currentFileName == "") {
        return;
    }
    QDesktopServices::openUrl(QFileInfo(currentFileName).absoluteDir().path());
}

void MainWindow::openEditor()
{
    if (currentFileName == "") {
        return;
    }
    QDesktopServices::openUrl(currentFileName);
}

void MainWindow::stopActions() { worker.interrupt(); }
