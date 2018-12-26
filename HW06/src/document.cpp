#include "include/document.h"
#include <QDebug>

#include <QSet>

size_t Document::QStringHash::operator()(QString const &target) const
{
    return qHash(target);
}
