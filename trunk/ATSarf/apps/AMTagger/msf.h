#ifndef MSF_H
#define MSF_H

#include <QString>
#include <QTreeWidget>
#include <qjson/parser.h>
#include "nfa.h"

class MSF
{
public:
    MSF(QString name, MSF* parent);
    QString name;
    QString actions;
    MSF * parent;
    virtual bool isMBF()=0;
    virtual bool isUnary()=0;
    virtual bool isBinary()=0;
    virtual bool isFormula()=0;
    virtual bool isSequential()=0;
    virtual QString print()=0;
    virtual void buildTree(QTreeWidgetItem* parent)=0;
    virtual void buildTree(QTreeWidget* parent)=0;
    virtual QVariantMap getJSON()=0;
    virtual bool buildNFA(NFA* nfa)=0;
    virtual ~MSF();
};

#endif // MSF_H
