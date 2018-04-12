#ifndef TAGTYPELEGEND_H
#define TAGTYPELEGEND_H
#include <QtGui>
#include "Tag.h"
#include "legend.h"
#include "TagType.h"
class TagtypeLegend
{
public:
Legend GetLegend (const TagType &); //each tagtype should have one legend
TagType GetTags ( Legend &); // each legend can have many tagtypes especially if some tagtypes are subsets of bigger tagtype
bool Remove(TagType &, Legend &);
bool Add(TagType &, Legend &);
 //  bool operator==(QMap<TagType,Legend>);
private:
 QMap<TagType,Legend> LegTag;

};
#endif // TAGTYPELEGEND_H
