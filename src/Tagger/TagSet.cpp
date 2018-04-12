#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TagSet.h"


TagSet::TagSet()
{
}

TagType* TagSet::GetTags (int start, int length)
{
  for (int i=0;i<TagVector.size(); i++)
    { if ((start== TagVector[i].GetStart()) && (length==TagVector[i].GetLength()))
return TagVector[i].GetTagType();
    }


    return new TagType(); //not sure if it is a safe way

}


bool TagSet::AddTag (Tag &tag)
{
TagVector.append(tag);
    return 1;
}

bool TagSet::RemoveTag (Tag &tag)
{int i=0;
  for ( i=0;i<TagVector.size(); i++)
      if (TagVector[i]==tag)
          break;
  i= TagVector.indexOf(tag,0);
    TagVector.remove(i);

return 1;
}
