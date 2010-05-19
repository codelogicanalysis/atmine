#ifndef TAGSET_H
#define TAGSET_H

#include <QtGui>
 #include <QVector>
#include "Tag.h"

class TagSet
{public:
 TagSet();
TagType* GetTags (int, int);
bool AddTag (Tag *);
bool RemoveTag (Tag *);

private:
 QVector <Tag> TagVector;
};
#endif // TAGSET_H
