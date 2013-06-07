#include "mbf.h"

MBF::MBF(QString name, QString bf, Operation op, int limit) : MSF(name)
{
    this->bf = bf;
    this->op = op;
    this->limit = limit;
}
