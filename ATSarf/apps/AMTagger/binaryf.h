#ifndef BINARYF_H
#define BINARYF_H

#include <QVector>
#include "msf.h"
#include "merfutil.h"

class BINARYF : public MSF
{
public:
    BINARYF(QString name, MSF* parent, Operation op);
    void setLeftMSF(MSF*);
    void setRightMSF(MSF*);
    MSF* leftMSF;
    MSF* rightMSF;
    Operation op;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
    QString printwithNames();
    void buildTree(QTreeWidgetItem* parent);
    void buildTree(QTreeWidget* parent);
    bool buildActionFile(QString &actionsData, QMultiMap<QString, QString> *functionParametersMap);
    QVariantMap getJSON();
    bool buildNFA(NFA *nfa);
    bool removeSelfFromMap(QMap<QString, MSF*> &map);
    QStringList getMSFNames();
    ~BINARYF();
};

#endif // BINARYF_H
