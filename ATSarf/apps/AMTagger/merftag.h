#ifndef MERFTAG_H
#define MERFTAG_H

#include <QVector>
#include "tag.h"

class MERFTag : public Tag
{
public:
    MERFTag();
    MERFTag(QString name, int pos, int length);
    QVector<Tag*> *tags;
};

#endif // MERFTAG_H
