#ifndef MSF_H
#define MSF_H

#include <QString>

class MSF
{
public:
    MSF(QString name, MSF* parent);
    QString name;
    QString actions;
    MSF * parent;
    virtual bool isMBF()=0;
    virtual bool isUnary()=0;
    virtual bool isBinary()=0;
    virtual bool isFormula()=0;
    virtual bool isSequential()=0;
    virtual QString print()=0;
};

#endif // MSF_H
