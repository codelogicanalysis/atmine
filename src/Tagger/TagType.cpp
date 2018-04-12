#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TagType.h"


TagType::TagType()
{
    ID=-1;
    type = "";
    description = "";
    source="";
}

TagType::TagType(int i, QString t, QString d, QString s)
{
    ID = i;
    type = t;
    description = d;
    source=s;
}

void TagType::setID(int i)
{
    ID = i;
}

void TagType::setType(QString t)
{
    type = t;
}

void TagType::setDescription(QString d)
{
    description = d;
}

int TagType::getID()
{
    return ID;
}

QString TagType::getType()
{
    return type;
}

QString TagType::getDescription()
{
    return description;
}

void TagType::setSource(QString s)
{source=s;
}
QString TagType::getSource()
{
    return source;
}
bool TagType::operator== (TagType &t2)
{
 if ((t2.description==description) && (t2.ID==ID) && (t2.source==source) &&(t2.type==type))

    return 1;
 else
     return 0;

}
/*
bool TagType::operator< (const TagType &t2)
{
    if (t2.ID < ID)
        return 1;
    else return 0;
}
*/
bool TagType::operator> (const TagType &t2)
{

    if (t2.ID > ID)
        return 1;
    else return 0;
}


bool TagType::operator< ( TagType &t2)
{
    if (t2.ID < ID)
        return 1;
    else return 0;
}

bool TagType::operator> ( TagType &t2)
{

    if (t2.ID > ID)
        return 1;
    else return 0;
}
