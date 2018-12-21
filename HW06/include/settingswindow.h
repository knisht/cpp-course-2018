#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <memory>

namespace Ui
{
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    void setValues(bool asyncSearch, bool liveColoring);
    ~SettingsWindow();

signals:
    void sendSettings(bool asyncSearch, bool liveColoring);

public slots:
    void acceptSettings();

private:
    std::unique_ptr<Ui::SettingsWindow> ui;
};

#endif // SETTINGSWINDOW_H
