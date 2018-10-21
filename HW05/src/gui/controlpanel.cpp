#include "include/gui/controlpanel.h"
#include <QHBoxLayout>
namespace gui
{
ControlPanel::ControlPanel()
    : layout(new QVBoxLayout), searchForEveryButton(), removeButton(),
      searchForOneButton()
{
    searchForEveryButton.setText("Find equal for every file");
    connect(&searchForEveryButton, SIGNAL(pressed()), this,
            SLOT(everyFileEmitter()));
    removeButton.setText("Remove file");
    connect(&removeButton, SIGNAL(pressed()), this, SLOT(removeFileEmitter()));
    searchForOneButton.setText("Find equal for one file");
    connect(&searchForOneButton, SIGNAL(pressed()), this,
            SLOT(oneFileEmitter()));
    goToParentButton.setText("Go to parent directory");
    connect(&goToParentButton, SIGNAL(pressed()), this, SLOT(goUpEmitter()));
    goToInnerDirButton.setText("Change root directory");
    connect(&goToInnerDirButton, SIGNAL(pressed()), this,
            SLOT(goDownEmitter()));
    firstLine.setFrameShape(QFrame::HLine);
    secondLine.setFrameShape(QFrame::HLine);
    firstLine.setFrameShadow(QFrame::Sunken);
    secondLine.setFrameShadow(QFrame::Sunken);
    layout->addWidget(&searchForEveryButton);
    layout->addWidget(&searchForOneButton);
    layout->addWidget(&firstLine);
    layout->addWidget(&removeButton);
    layout->addWidget(&secondLine);
    layout->addWidget(&goToParentButton);
    layout->addWidget(&goToInnerDirButton);
    setLayout(layout);
}

ControlPanel::~ControlPanel() { delete layout; }

void ControlPanel::everyFileEmitter() { emit splitEverything(); }
void ControlPanel::oneFileEmitter() { emit splitForOne(); }
void ControlPanel::removeFileEmitter() { emit removeFiles(); }
void ControlPanel::goUpEmitter() { emit goUpper(); }
void ControlPanel::goDownEmitter() { emit goDeeper(); }
} // namespace gui
