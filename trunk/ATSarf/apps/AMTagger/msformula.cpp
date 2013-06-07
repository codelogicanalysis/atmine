#include "msformula.h"

MSFormula::MSFormula(QString name)
{
    this->name = name;
}

void MSFormula::addMSF(QString subname, MSF * msf) {
    map.insert(subname,msf);
    vector.append(msf);
}
