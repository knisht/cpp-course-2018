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
                      ui->onlineColoringButton->isChecked());
    close();
}

void SettingsWindow::setValues(bool asyncSearch, bool liveColoring)
{
    ui->asyncSearchButton->setChecked(asyncSearch);
    ui->onlineColoringButton->setChecked(liveColoring);
}

SettingsWindow::~SettingsWindow() {}
