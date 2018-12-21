#ifndef LIBRARIAN_SUBSTRING_OCCURRENCE_H
#define LIBRARIAN_SUBSTRING_OCCURRENCE_H

#include <QEventLoop>

class SubstringOccurrence;

Q_DECLARE_METATYPE(SubstringOccurrence)

class SubstringOccurrence : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE SubstringOccurrence()
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence(QString const &string, size_t id)
        : filename(string), id(id)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence(SubstringOccurrence const &other)
        : filename(other.filename), id(other.id)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence operator=(SubstringOccurrence const &other)
    {
        qRegisterMetaType<SubstringOccurrence>();
        filename = other.filename;
        id = other.id;
        return *this;
    }

    QString filename;
    size_t id;

    friend bool operator==(SubstringOccurrence const &a,
                           SubstringOccurrence const &b)
    {
        return a.id == b.id && a.filename == b.filename;
    }
};

#endif
