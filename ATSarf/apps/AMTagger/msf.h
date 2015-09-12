#ifndef MSF_H
#define MSF_H

#include <QString>
#include <QTreeWidget>
#include "nfa.h"

class NFA;

class MSF
{
public:
    MSF(QString name, MSF* parent);
    QString name;
    QString init;
    QString actions;
    //QString after;
    QString returns;
    MSF * parent;
    virtual bool isMBF()=0;
    virtual bool isUnary()=0;
    virtual bool isBinary()=0;
    virtual bool isFormula()=0;
    virtual bool isSequential()=0;
    virtual QString print()=0;
    virtual QString printwithNames()=0;
    virtual void buildTree(QTreeWidgetItem* parent)=0;
    virtual void buildTree(QTreeWidget* parent)=0;
    virtual bool buildActionFile(QString &actionsData, QMultiMap<QString, QPair<QString,QString> > *functionParametersMap)=0;
    virtual QVariantMap getJSON()=0;
    virtual bool buildNFA(NFA* nfa)=0;
    virtual bool removeSelfFromMap(QMap<QString, MSF*> &map)=0;
    virtual QStringList getMSFNames()=0;
    virtual ~MSF();
};

#endif // MSF_H
