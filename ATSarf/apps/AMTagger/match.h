#ifndef MATCH_H
#define MATCH_H

#include <cstddef>
#include <sstream>
#include <string.h>
#include <math.h>
#include <QTextStream>
#include "merfutil.h"
#include "msformula.h"
#include "graphviz/cgraph.h"
#include "graphviz/gvc.h"
#include "numnorm.h"
using namespace std;

class Match
{
public:
    Match(Operation op, Match *parent);
    Match *parent;
    MSF* msf;
    Operation op;
    virtual bool setMatch(Match* match)=0;
    virtual bool isUnaryM()=0;
    virtual bool isBinaryM()=0;
    virtual bool isSequentialM()=0;
    virtual bool isKeyM()=0;
    virtual bool isMERFTag()=0;
    virtual int getPOS()=0;
    virtual int getLength()=0;
    virtual QString getText()=0;
    virtual int getMatchCount()=0;
    virtual void buildMatchTree(Agraph_t* G,Agnode_t* node,Agedge_t* edge,QMap<Agnode_t *,Agnode_t *>* parentNodeMap,QTreeWidgetItem* parentItem,int& id)=0;
    virtual void executeActions(NFA* nfa)=0;
    virtual QString getParam(QString msfName,QString param, QString* sarfMatches=NULL)=0;
    virtual QVariantMap getJSON()=0;
    virtual bool constructRelation(Relation* relation, Match*& entity1, Match*& entity2, Match*& edge)=0;
    virtual ~Match();
};

#endif // MATCH_H
