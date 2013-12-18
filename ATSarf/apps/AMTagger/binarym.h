#ifndef BINARYM_H
#define BINARYM_H

#include "match.h"

class BinaryM : public Match
{
public:
    BinaryM(Operation op, Match* parent);
    Match *leftMatch;
    Match *rightMatch;
    bool setMatch(Match* match);
    bool isUnaryM();
    bool isBinaryM();
    bool isSequentialM();
    bool isKeyM();
    bool isMERFTag();
    int getPOS();
    int getLength();
    QString getText();
    int getMatchCount();
    void buildMatchTree(Agraph_t* G,Agnode_t* node,Agedge_t* edge,QMap<Agnode_t *,Agnode_t *>* parentNodeMap,QTreeWidgetItem* parentItem, int& id);
    void executeActions(NFA* nfa);
    QString getParam(QString msfName,QString param, QString* sarfMatches=NULL);
    QVariantMap getJSON();
    bool constructRelation(Relation* relation, Match*& entity1, Match*& entity2, Match*& edge);
    ~BinaryM();
};

#endif // BINARYM_H
