#include "include/textviewdriver.h"

#include <QDebug>
#include <QFile>

TextViewDriver::TextViewDriver(QWidget *&parent) : QTextEdit(parent)
{
    occurrenceIndex = -1;
    currentFileName = "";
}

TextViewDriver::~TextViewDriver() {}

bool TextViewDriver::loadText(QString const &path)
{
    currentFileName = path;
    QFile file(currentFileName);
    QTextEdit::clear();
    if (file.open(QFile::ReadOnly)) {
        QTextEdit::document()->setPlainText(file.readAll());
        defaultCursor = QTextEdit::textCursor();
        return true;
    } else {
        QTextEdit::document()->setPlainText("Something wrong with this file");
        return false;
    }
}

void TextViewDriver::setWordSize(qsizetype newSize) { wordSize = newSize; }

void TextViewDriver::setWordPositions(std::vector<size_t> &&word_positions)
{
    currentWordPositionsInFile = word_positions;
}

void TextViewDriver::renderText()
{
    if (currentFileName == "" || currentWordPositionsInFile.size() == 0) {
        return;
    }
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::Document);
    tc.setCharFormat(defaultCursor.charFormat());
    QTextEdit::setTextCursor(defaultCursor);
    QTextCharFormat fmt;
    fmt.setBackground(QColor{0, 204, 0, 200});
    QTextEdit::textCursor().clearSelection();

    QTextCursor cursor(QTextEdit::textCursor());
    cursor.setCharFormat(fmt);
    for (size_t position : currentWordPositionsInFile) {
        cursor.setPosition(static_cast<int>(position), QTextCursor::MoveAnchor);
        cursor.setPosition(static_cast<int>(position) +
                               static_cast<int>(wordSize),
                           QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt);
    }
    QTextEdit::setTextCursor(cursor);
    occurrenceIndex = 0;
    highlightCurrentOccurrence();
}

void TextViewDriver::interrupt() {}
void TextViewDriver::flush() { currentWordPositionsInFile.clear(); }

void TextViewDriver::establishNextOccurrence()
{
    if (occurrenceIndex == -1 || currentWordPositionsInFile.size() == 0) {
        return;
    }

    occurrenceIndex =
        static_cast<qsizetype>(static_cast<size_t>(occurrenceIndex + 1) %
                               currentWordPositionsInFile.size());
    highlightCurrentOccurrence();
}

void TextViewDriver::establishPrevOccurrence()
{
    if (occurrenceIndex == -1 || currentWordPositionsInFile.size() == 0) {
        return;
    }

    occurrenceIndex = static_cast<qsizetype>(
        static_cast<size_t>(static_cast<size_t>(occurrenceIndex) +
                            currentWordPositionsInFile.size() - 1) %
        currentWordPositionsInFile.size());
    highlightCurrentOccurrence();
}

void TextViewDriver::highlightCurrentOccurrence()
{
    QTextCursor textCursor(QTextEdit::textCursor());
    int position = static_cast<int>(
        currentWordPositionsInFile[static_cast<size_t>(occurrenceIndex)]);
    textCursor.setPosition(position, QTextCursor::MoveAnchor);
    textCursor.setPosition(position + static_cast<int>(wordSize),
                           QTextCursor::KeepAnchor);
    QTextEdit::setTextCursor(textCursor);
}

QString const &TextViewDriver::getCurrentFilename() const
{
    return currentFileName;
}
