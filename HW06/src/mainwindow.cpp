#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      settingsWindow(new SettingsWindow(this)), currentDir("."),
      settings("Zeron_Software", "Librarian")
{
    ui->setupUi(this);
    setWindowTitle("Librarian");
    ui->label->setText("Current dir: " + QDir(".").canonicalPath());
    connect(&worker, SIGNAL(startedIndexing()), this,
            SLOT(onStartedIndexing()));
    connect(&worker, SIGNAL(finishedIndexing(QString const &)), this,
            SLOT(onFinishedIndexing(QString const &)));
    connect(&worker, SIGNAL(startedFinding()), this, SLOT(onStartedFinding()));
    connect(&worker, SIGNAL(finishedFinding(QString const &)), this,
            SLOT(onFinishedFinding(QString const &)));
    connect(&worker, SIGNAL(properFileFound(SubstringOccurrence const &)), this,
            SLOT(getOccurrence(SubstringOccurrence const &)));
    connect(&worker, SIGNAL(determinedFilesAmount(qint64)), this,
            SLOT(setProgressBarLimit(qint64)));
    connect(&worker, SIGNAL(progressChanged(qint64)), this,
            SLOT(changeProgressBarValue(qint64)));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(showSettings()));
    connect(settingsWindow.get(), SIGNAL(sendSettings(bool, bool, bool)), this,
            SLOT(receiveSettings(bool, bool, bool)));
    ui->filesContent->setCursorWidth(0);
    ui->filesContent->setAcceptRichText(false);
    indexate(".");
}

void MainWindow::findSubstring()
{
    if (isIndexing) {
        return;
    }
    currentWord = ui->stringInput->toPlainText();
    if (currentWord.size() == 0) {
        return;
    }
    ui->filesContent->setWordSize(currentWord.size());
    worker.findSubstringAsync(
        currentWord,
        settings.value("behavior/parallelSearch", false).value<bool>());
    ui->filesContent->flush();
    ui->filesWidget->clear();
}

void MainWindow::getFileContent(QListWidgetItem *item)
{
    QString filename = QDir(currentDir).absoluteFilePath(item->text());
    qInfo() << "Opening" << filename;
    ui->currentFileLabel->setText("Opened file: " +
                                  QFileInfo(filename).fileName());
    ui->filesContent->loadText(QDir(currentDir).absoluteFilePath(filename));
    ui->filesContent->setWordSize(currentWord.size());
    ui->filesContent->setWordPositions(
        worker.getFileStat(filename, currentWord));
    ui->filesContent->renderText();
}

void MainWindow::changeDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Open directory", currentDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    qInfo() << "Directory chosen:" << dir;
    if (dir != "") {
        indexate(dir);
        currentDir = QDir(dir).absolutePath();
        ui->label->setText("Current dir: " + currentDir);
    }
}

MainWindow::~MainWindow() { worker.interrupt(); }

void MainWindow::onStartedIndexing()
{
    ui->progressBar->reset();
    isIndexing = true;
    ui->statusbar->showMessage("Indexing...");
}

void MainWindow::onFinishedIndexing(QString const &result)
{
    isIndexing = false;
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

void MainWindow::getOccurrence(SubstringOccurrence const &newFile)
{
    //    ui->progressBar->setValue(ui->progressBar->value() + 500);
    //    NOTE: better to keep above lines commented, otherwise program
    //    performance is much slower
    if (worker.validate(newFile)) {
        ui->filesWidget->addItem(
            QDir(currentDir).relativeFilePath(newFile.filename));
    }
    if (settings.value("behavior/liveColoring").value<bool>() &&
        newFile.filename == ui->filesContent->getCurrentFilename()) {
        if (QFileInfo(ui->filesContent->getCurrentFilename()).size() > 800000 &&
            currentWord.size() < 3) {
            ui->statusbar->showMessage(
                "File is too big to render it; Try to use bigger patterns");
        } else {
            ui->filesContent->setWordPositions(worker.getFileStat(
                ui->filesContent->getCurrentFilename(), currentWord));
            ui->filesContent->setWordSize(currentWord.size());
            ui->filesContent->renderText();
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

void MainWindow::receiveSettings(bool asyncSearch, bool liveColoring,
                                 bool fileWatching)
{
    settings.setValue("behavior/parallelSearch", asyncSearch);
    settings.setValue("behavior/liveColoring", liveColoring);
    settings.setValue("behavior/fileWatching", fileWatching);
}

void MainWindow::openFileManager()
{
    if (ui->filesContent->getCurrentFilename() == "") {
        return;
    }
    QDesktopServices::openUrl(
        QFileInfo(ui->filesContent->getCurrentFilename()).absoluteDir().path());
}

void MainWindow::openEditor()
{
    if (ui->filesContent->getCurrentFilename() == "") {
        return;
    }
    QDesktopServices::openUrl(ui->filesContent->getCurrentFilename());
}

void MainWindow::indexate(QString const &path)
{
    worker.indexateAsync(
        path, settings.value("behavior/fileWatching", false).value<bool>());
}

void MainWindow::stopActions() { worker.interrupt(); }

void MainWindow::showSettings()
{
    settingsWindow->show();
    settingsWindow->setValues(
        settings.value("behavior/parallelSearch", false).value<bool>(),
        settings.value("behavior/liveColoring", false).value<bool>(),
        settings.value("behavior/fileWatching", false).value<bool>());
}
