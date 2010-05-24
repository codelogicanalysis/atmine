#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TagtypeLegend.h"
#include <QtCore>
 #include <QFuture>
 #include <Q3ValueList>

Legend TagtypeLegend::GetLegend (const TagType &tagtype)
{
return LegTag.value(tagtype);
}

TagType TagtypeLegend::GetTags (Legend &legnd)
{
 //LegTag.key(legnd);

    QList <TagType> tt= LegTag.keys();
    if (tt.size()==0)
    { TagType l1(-1,"","","");
    return l1;}

    for (int i=0;i<tt.size();i++)
    if (GetLegend(tt[i])==legnd)
        return tt[i];
}

bool TagtypeLegend::Add(TagType &t1, Legend &l1)
{
if (LegTag.contains(t1))
    return 1;

    LegTag.insert(t1,l1);
      return 1;

}

bool TagtypeLegend::Remove(TagType &t1, Legend &l1)
{if (LegTag.contains(t1))
   LegTag.remove(t1);
    return 1;

}
