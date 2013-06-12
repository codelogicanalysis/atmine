#ifndef MBF_H
#define MBF_H

#include "msf.h"
#include "merfutil.h"

class MBF : public MSF
{
public:
    MBF(QString name, MSF* parent, QString bf);
    QString bf;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
};

#endif // MBF_H
