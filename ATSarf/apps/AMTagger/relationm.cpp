#include "relationm.h"

RelationM::RelationM(Relation *relation, Match *entity1, QString e1Label, Match *entity2, QString e2Label, Match *edge, QString edgeLabel)
{
    this->relation = relation;
    this->entity1 = entity1;
    this->e1Label = e1Label;
    this->entity2 = entity2;
    this->e2Label = e2Label;
    this->edge = edge;
    this->edgeLabel = edgeLabel;
}

RelationM::RelationM(Relation *relation, Match *entity1, Match *entity2, Match *edge)
{
    this->relation = relation;
    this->entity1 = entity1;
    if(relation->e1Label == "text") {
        this->e1Label = entity1->getText();
    }
    else if(relation->e1Label == "position") {
        this->e1Label = QString::number(entity1->getPOS());
    }
    else if(relation->e1Label == "length") {
        this->e1Label = QString::number(entity1->getLength());
    }
    else if(relation->e1Label == "number") {
        QString text = entity1->getText();
        NumNorm nn(&text);
        nn();
        if(nn.extractedNumbers.count()!=0) {
            int number = nn.extractedNumbers[0].getNumber();
            this->e1Label = QString::number(number);
        }
        else {
            this->e1Label = "";
        }
    }
    this->entity2 = entity2;
    if(relation->e2Label == "text") {
        this->e2Label = entity2->getText();
    }
    else if(relation->e2Label == "position") {
        this->e2Label = QString::number(entity2->getPOS());
    }
    else if(relation->e2Label == "length") {
        this->e2Label = QString::number(entity2->getLength());
    }
    else if(relation->e2Label == "number") {
        QString text = entity2->getText();
        NumNorm nn(&text);
        nn();
        if(nn.extractedNumbers.count()!=0) {
            int number = nn.extractedNumbers[0].getNumber();
            this->e2Label = QString::number(number);
        }
        else {
            this->e2Label = "";
        }
    }
    this->edge = edge;
    if(edge == NULL) {
        this->edgeLabel = relation->edgeLabel;
    }
    else {
        if(relation->edgeLabel == "text") {
            this->edgeLabel = edge->getText();
        }
        else if(relation->edgeLabel == "position") {
            this->edgeLabel = QString::number(edge->getPOS());
        }
        else if(relation->edgeLabel == "length") {
            this->edgeLabel = QString::number(edge->getLength());
        }
        else if(relation->edgeLabel == "number") {
            QString text = edge->getText();
            NumNorm nn(&text);
            nn();
            if(nn.extractedNumbers.count()!=0) {
                int number = nn.extractedNumbers[0].getNumber();
                this->edgeLabel = QString::number(number);
            }
            else {
                this->edgeLabel = "";
            }
        }
    }
}
