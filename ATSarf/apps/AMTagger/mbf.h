#ifndef MBF_H
#define MBF_H

#include "msf.h"
#include "merfutil.h"

class MBF : public MSF
{
public:
    MBF(QString, QString, Operation, int);
    QString bf;
    Operation op;
    int limit;
};

#endif // MBF_H
