#ifndef UNARYF_H
#define UNARYF_H

#include <QVector>
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
};

#endif // UNARYF_H
