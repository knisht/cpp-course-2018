#include "include/settingswindow.h"
#include "ui_settingswindow.h"
#include <QDebug>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowTitle("Settings");
}

void SettingsWindow::acceptSettings()
{
    QMap<QString, bool> currentSettings;
    currentSettings["parallelSearching"] = ui->asyncSearchButton->isChecked();
    currentSettings["liveColoring"] = ui->onlineColoringButton->isChecked();
    currentSettings["fileWatching"] = ui->fileWatchingButton->isChecked();
    currentSettings["liveSearching"] = ui->liveSearchButton->isChecked();
    emit sendSettings(currentSettings);
    close();
}

void SettingsWindow::setValues(QMap<QString, bool> values)
{
    ui->asyncSearchButton->setChecked(values.value("parallelSearching", false));
    ui->onlineColoringButton->setChecked(values.value("liveColoring", false));
    ui->fileWatchingButton->setChecked(values.value("fileWatching", false));
    ui->liveSearchButton->setChecked(values.value("liveSearching", false));
}

SettingsWindow::~SettingsWindow() {}
