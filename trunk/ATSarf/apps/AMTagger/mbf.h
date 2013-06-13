#ifndef MBF_H
#define MBF_H

#include "msf.h"
#include "merfutil.h"

class MBF : public MSF
{
public:
    MBF(QString name, MSF* parent, QString bf, bool isF = false);
    QString bf;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
    void buildTree(QTreeWidgetItem* parent);
    void buildTree(QTreeWidget* parent);
    QVariantMap getJSON();
    //QVariantList getJSON();
    bool isF;
    ~MBF();
};

#endif // MBF_H
