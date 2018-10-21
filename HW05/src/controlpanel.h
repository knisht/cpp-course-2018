#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QPushButton>
#include <QWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    ControlPanel();
    ~ControlPanel() override;
    QPushButton const &get_button();
signals:

public slots:

private:
    QLayout *layout;
    QPushButton button;
};

#endif // CONTROLPANEL_H
