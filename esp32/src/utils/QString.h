#ifndef QSTRING_H
#define QSTRING_H

#include <string>

#include "QByteArray.h"

class QString;

typedef std::vector<QString> QStringList;

class QString : public std::string
{
public:
    QString();
    QString(const QByteArray& srcArray);
    QString(const std::string& srcString);

    QStringList split(const char* separator);
};

#endif