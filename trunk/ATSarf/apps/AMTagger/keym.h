#ifndef KEYM_H
#define KEYM_H

#include <QString>
#include "match.h"

class KeyM : public Match
{
public:
    KeyM(Match* parent, QString key, int pos, int length);
    QString word;
    QString key;
    int pos;
    int length;
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
    QString getParam(QString msfName,QString param);
    QVariantMap getJSON();
    ~KeyM();
};

#endif // KEYM_H
