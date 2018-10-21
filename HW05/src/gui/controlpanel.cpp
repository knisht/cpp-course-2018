#include "../include/controlpanel.h"
#include <QHBoxLayout>

ControlPanel::ControlPanel()
    : layout(new QVBoxLayout), addButton(), removeButton(), searchForOneButton()
{
    addButton.setText("Find equal for every file");
    removeButton.setText("Remove file");
    searchForOneButton.setText("Find equal for one file");
    goToParentButton.setText("Go to parent directory");
    goToInnerDirButton.setText("Change root directory");
    firstLine.setFrameShape(QFrame::HLine);
    secondLine.setFrameShape(QFrame::HLine);
    firstLine.setFrameShadow(QFrame::Sunken);
    secondLine.setFrameShadow(QFrame::Sunken);
    layout->addWidget(&addButton);
    layout->addWidget(&searchForOneButton);
    layout->addWidget(&firstLine);
    layout->addWidget(&removeButton);
    layout->addWidget(&secondLine);
    layout->addWidget(&goToParentButton);
    layout->addWidget(&goToInnerDirButton);
    setLayout(layout);
}

ControlPanel::~ControlPanel() { delete layout; }

QPushButton const &ControlPanel::get_add_button() { return addButton; }
QPushButton const &ControlPanel::get_remove_button() { return removeButton; }
QPushButton const &ControlPanel::get_search_for_one_button()
{
    return searchForOneButton;
}

QPushButton const &ControlPanel::go_to_parent_button()
{
    return goToParentButton;
}

QPushButton const &ControlPanel::go_to_inner_dir_button()
{
    return goToInnerDirButton;
}
