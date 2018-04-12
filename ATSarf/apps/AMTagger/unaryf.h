#ifndef UNARYF_H
#define UNARYF_H

#include <QVector>
#include <QTreeWidget>
#include "msf.h"
#include "merfutil.h"

class UNARYF : public MSF
{
public:
    UNARYF(QString name, MSF* parent, Operation op, int limit=-1);
    void setMSF(MSF* msf);
    MSF* msf;
    Operation op;
    int limit;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
    QString printwithNames();
    void buildTree(QTreeWidgetItem* parent);
    void buildTree(QTreeWidget* parent);
    bool buildActionFile(QString &actionsData, QMultiMap<QString, QPair<QString,QString> > *functionParametersMap);
    QVariantMap getJSON();
    bool buildNFA(NFA *nfa);
    bool removeSelfFromMap(QMap<QString, MSF*> &map);
    QStringList getMSFNames();
    ~UNARYF();
};

#endif // UNARYF_H
