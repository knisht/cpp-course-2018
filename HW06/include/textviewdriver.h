#ifndef LIBRARIAN_TEXTVIEWDRIVER_H
#define LIBRARIAN_TEXTVIEWDRIVER_H
#include <QTextEdit>

class TextViewDriver : public QTextEdit
{
    Q_OBJECT
public:
    TextViewDriver(QWidget *&);
    virtual ~TextViewDriver();

    void setWordPositions(std::vector<size_t> &&word_positions);
    void setWordSize(qsizetype);

    void flush();
public slots:
    void establishNextOccurrence();
    void establishPrevOccurrence();
    bool loadText(QString const &path);
    void renderText();
    void interrupt();
    QString const &getCurrentFilename() const;

private:
    void highlightCurrentOccurrence();

    QString currentFileName;
    qsizetype occurrenceIndex;
    QTextCursor defaultCursor;
    qsizetype wordSize;
    std::vector<size_t> currentWordPositionsInFile;
};

#endif
