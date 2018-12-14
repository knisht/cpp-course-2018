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

    Q_INVOKABLE SubstringOccurrence(QString const &string,
                                    std::vector<size_t> vec)
        : filename(string), occurrences(vec)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence(SubstringOccurrence const &other)
        : filename(other.filename), occurrences(other.occurrences)
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    Q_INVOKABLE SubstringOccurrence operator=(SubstringOccurrence const &other)
    {
        qRegisterMetaType<SubstringOccurrence>();
        filename = other.filename;
        occurrences = other.occurrences;
        return *this;
    }

    Q_INVOKABLE SubstringOccurrence(QString const &filename)
        : filename(filename), occurrences()
    {
        qRegisterMetaType<SubstringOccurrence>();
    }

    QString filename;
    std::vector<size_t> occurrences;

    friend bool operator==(SubstringOccurrence const &a,
                           SubstringOccurrence const &b)
    {
        return a.filename == b.filename && a.occurrences == b.occurrences;
    }
};

#endif
