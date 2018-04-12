#ifndef TAGTYPE_H
#define TAGTYPE_H

#include <QtGui>

class TagType
{public:
    int ID;
    QString type;
    QString description;
    QString source; // 0: automatic, 1: user entry

public:
    TagType(int,QString,QString,QString);
    TagType();
    void setID(int);
    void setType(QString);
    void setDescription(QString);
    void setSource(QString);
    int getID();
    QString getSource();
    QString getType();
    QString getDescription();
    bool operator ==(TagType &);
    //bool operator< (const TagType &);
    bool operator> (const TagType &);

    bool operator< (TagType &);
    bool operator> (TagType &);
};

inline bool operator<(const TagType &t1, const TagType &t2)

{

    if ( t1.ID < t2.ID)
        return 1;
    else return 0;
}


#endif // TAGTYPE_H
