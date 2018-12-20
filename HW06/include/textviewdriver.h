#ifndef LIBRARIAN_TEXTVIEWDRIVER_H
#define LIBRARIAN_TEXTVIEWDRIVER_H
#include <QTextEdit>

class TextViewDriver : public QTextEdit
{
    Q_OBJECT
public:
    TextViewDriver() {}
    virtual ~TextViewDriver() {}
};

#endif
