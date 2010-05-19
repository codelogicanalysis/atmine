#ifndef TAGTYPE_H
#define TAGTYPE_H

#include <QtGui>

class TagType
{private:
    int ID;
    QString type;
    QString description;
    QString source; // 0: automatic, 1: user entry

public:
    TagType(int,QString,QString,QString);
    void setID(int);
    void setType(QString);
    void setDescription(QString);
    void setSource(QString);
    int getID();
    QString getSource();
    QString getType();
    QString getDescription();
};



#endif // TAGTYPE_H
