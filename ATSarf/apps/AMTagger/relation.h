#ifndef RELATION_H
#define RELATION_H

#include "msf.h"

class Relation
{
public:
    Relation();
    Relation(QString name, MSF* entity1, QString e1Label, MSF* entity2, QString e2Label, MSF* edge, QString edgeLabel);
    QString name;
    MSF* entity1;
    QString e1Label;
    MSF* entity2;
    QString e2Label;
    MSF* edge;
    QString edgeLabel;
};

#endif // RELATION_H
