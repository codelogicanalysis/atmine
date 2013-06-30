#ifndef SEQUETIALF_H
#define SEQUETIALF_H
#include <QVector>
#include "msf.h"

class SequentialF : public MSF
{
public:
    SequentialF(QString name, MSF* parent);
    void addMSF(MSF* msf);
    QVector<MSF*> vector;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
    void buildTree(QTreeWidgetItem* parent);
    void buildTree(QTreeWidget* parent);
    QVariantMap getJSON();
    bool buildNFA(NFA *nfa);
    bool removeSelfFromMap(QMap<QString, MSF*> &map);
    ~SequentialF();
};

#endif // SEQUETIALF_H
