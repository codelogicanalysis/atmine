#ifndef TAGTYPE_H
#define TAGTYPE_H

#include<QString>

class TagType
{
public:
    TagType();
    TagType(QString, QString, int, QString, QString, int, bool, bool, bool);
//private:
    QString tag;
    QString description;
    int id;
    QString fgcolor;
    QString bgcolor;
    int font;
    bool underline;
    bool bold;
    bool italic;
};

#endif // TAGTYPE_H
