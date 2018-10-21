#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    ControlPanel();
    ~ControlPanel() override;
    QPushButton const &get_add_button();
    QPushButton const &get_remove_button();
    QPushButton const &get_search_for_one_button();
    QPushButton const &go_to_parent_button();
    QPushButton const &go_to_inner_dir_button();

private:
    QLayout *layout;
    QFrame firstLine;
    QFrame secondLine;
    QPushButton addButton;
    QPushButton removeButton;
    QPushButton searchForOneButton;
    QPushButton goToParentButton;
    QPushButton goToInnerDirButton;
};

#endif // CONTROLPANEL_H
