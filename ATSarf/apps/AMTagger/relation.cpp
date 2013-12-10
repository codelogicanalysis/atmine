#include "relation.h"

Relation::Relation() {
    entity1 = NULL;
    entity2 = NULL;
    edge = NULL;
}

Relation::Relation(QString name, MSF *entity1, QString e1Label, MSF *entity2, QString e2Label, MSF *edge, QString edgeLabel) {
    this->name = name;
    this->entity1 = entity1;
    this->e1Label = e1Label;
    this->entity2 = entity2;
    this->e2Label = e2Label;
    this->edge = edge;
    this->edgeLabel = edgeLabel;
}
