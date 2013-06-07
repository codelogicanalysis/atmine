#ifndef UNARYF_H
#define UNARYF_H

#include <QVector>
#include "msf.h"
#include "merfutil.h"

class UNARYF : public MSF
{
public:
    UNARYF(QString, Operation, int);
    void addMSF(MSF*);
    QVector<MSF*> vector;
    Operation op;
    int limit;
};

#endif // UNARYF_H
