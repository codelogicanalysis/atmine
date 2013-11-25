#ifndef TAGTYPE_H
#define TAGTYPE_H

#include<QString>
#include "commonS.h"

class TagType
{
public:
    TagType();
    TagType(QString, QString, QString, QString, int, bool, bool, bool, Source);
//private:
    Source source;
    QString name;
    QString description;
    QString fgcolor;
    QString bgcolor;
    int font;
    bool underline;
    bool bold;
    bool italic;
};

#endif // TAGTYPE_H
