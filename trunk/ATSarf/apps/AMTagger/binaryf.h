#ifndef BINARYF_H
#define BINARYF_H

#include <QVector>
#include "msf.h"
#include "merfutil.h"

class BINARYF : public MSF
{
public:
    BINARYF(QString, Operation);
    void addLeftMSF(MSF*);
    void addRightMSF(MSF*);
    QVector<MSF*> lvector;
    QVector<MSF*> rvector;
    Operation op;
};

#endif // BINARYF_H
