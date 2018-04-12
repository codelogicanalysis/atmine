#ifndef RELATIONM_H
#define RELATIONM_H

#include "match.h"

class RelationM
{
public:
    RelationM(Relation *relation, Match *entity1, Match *entity2, Match *edge);
    RelationM(Relation* relation,Match* entity1,QString e1Label,Match* entity2,QString e2Label,Match* edge,QString edgeLabel);
    Relation* relation;
    Match* entity1;
    QString e1Label;
    Match* entity2;
    QString e2Label;
    Match* edge;
    QString edgeLabel;
};

#endif // RELATIONM_H
