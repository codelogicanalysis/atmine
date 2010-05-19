#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TagSet.h"


TagSet::TagSet()
{//TagVector.reserve(200);
}

TagType* TagSet::GetTags (int start, int length)
{
    for (int i=0;i<TagVector.size(); i++)
        if ((start==TagVector[i].GetStart()) && (length==TagVector[i].GetLength()))
return TagVector[i].GetTagType();

   TagType fail1(-1,"","","");
    return &fail1; //not sure if it is a safe way

}


bool TagSet::AddTag (Tag* tag)
{Tag temp(tag->GetTagType(),tag->GetLength(),tag->GetStart());
TagVector.append(temp);
    return 1;
}

bool TagSet::RemoveTag (Tag *tag)
{Tag temp(tag->GetTagType(),tag->GetLength(),tag->GetStart());
   int i;
   i= TagVector.indexOf(temp);
    int vectorSize=TagVector.size();
    TagVector.remove(i);
    TagVector.resize(vectorSize-1);

return 1;
}
