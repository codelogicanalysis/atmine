#ifndef MSFORMULA_H
#define MSFORMULA_H

#include <QVector>
#include <QMap>
#include "msf.h"

class MSFormula
{
public:
    MSFormula(QString);
    void addMSF(QString, MSF*);
    /// Name of the formula
    QString name;
    /// Vector containing the sequence of MSFs
    QVector<MSF*> vector;
    /// Map that takes formula name and returns its structure
    QMap<QString,MSF*> map;
};

#endif // MSFORMULA_H
