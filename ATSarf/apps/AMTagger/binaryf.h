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
};

#endif // BINARYF_H
