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
    emit sendSettings(ui->asyncSearchButton->isChecked(),
                      ui->onlineColoringButton->isChecked(),
                      ui->fileWatchingButton->isChecked());
    close();
}

void SettingsWindow::setValues(bool asyncSearch, bool liveColoring,
                               bool fileWatching)
{
    ui->asyncSearchButton->setChecked(asyncSearch);
    ui->onlineColoringButton->setChecked(liveColoring);
    ui->fileWatchingButton->setChecked(fileWatching);
}

SettingsWindow::~SettingsWindow() {}
