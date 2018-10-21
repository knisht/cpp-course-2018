#include "controlpanel.h"
#include <QHBoxLayout>

ControlPanel::ControlPanel() : layout(new QHBoxLayout), button()
{
    button.setText("Find equal");
    layout->addWidget(&button);
    setLayout(layout);
}

ControlPanel::~ControlPanel() { delete layout; }

QPushButton const &ControlPanel::get_button() { return button; }
