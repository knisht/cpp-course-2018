#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QWidget>
#include <memory>

namespace gui
{
class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    ControlPanel();
    ~ControlPanel() override;
signals:
    void splitEverything();
    void removeFiles();
    void splitForOne();
    void goUpper();
    void goDeeper();
    void collapseAll();

private slots:
    void everyFileEmitter();
    void oneFileEmitter();
    void removeFileEmitter();
    void goUpEmitter();
    void goDownEmitter();
    void collapseAllEmitter();

private:
    std::unique_ptr<QLayout> layout;
    QFrame firstLine;
    QFrame secondLine;
    QPushButton searchForEveryButton;
    QPushButton removeButton;
    QPushButton searchForOneButton;
    QPushButton goToParentButton;
    QPushButton goToInnerDirButton;
    QPushButton collapseAllButton;
};

} // namespace gui
#endif // CONTROLPANEL_H
