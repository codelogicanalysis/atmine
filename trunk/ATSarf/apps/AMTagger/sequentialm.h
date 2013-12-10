#ifndef SEQUENTIALM_H
#define SEQUENTIALM_H

#include <QVector>
#include "match.h"

class SequentialM : public Match
{
public:
    SequentialM(Match* parent);
    QVector<Match*> matches;
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
    ~SequentialM();
};

#endif // SEQUENTIALM_H
