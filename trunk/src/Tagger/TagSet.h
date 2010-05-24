#ifndef TAGSET_H
#define TAGSET_H

#include <QtGui>
 #include <QVector>
#include "Tag.h"

class TagSet
{public:
TagSet();
TagType* GetTags (int, int);
bool AddTag (Tag &);
bool RemoveTag (Tag &);

private:
 QVector <Tag> TagVector; //here we will put each tagtype in the vector,
 //and if we want to remove or add a tag at a certain place we need
 // to search the tag

};
#endif // TAGSET_H
