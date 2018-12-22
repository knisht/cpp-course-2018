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
    void setValues(QMap<QString, bool>);
    ~SettingsWindow();

signals:
    void sendSettings(QMap<QString, bool>);

public slots:
    void acceptSettings();

private:
    std::unique_ptr<Ui::SettingsWindow> ui;
};

#endif // SETTINGSWINDOW_H
