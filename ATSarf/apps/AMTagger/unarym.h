#ifndef UNARYM_H
#define UNARYM_H

#include <QVector>
#include "match.h"

class UnaryM : public Match
{
public:
    UnaryM(Operation op, int id, Match *parent, int limit=-1);
    QVector<Match*> matches;
    int limit;
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
    void constructRelation(Relation* relation, QVector<Match*>& entity1, QVector<Match*>& entity2, QVector<Match*>& edge);
    ~UnaryM();
};

#endif // UNARYM_H
