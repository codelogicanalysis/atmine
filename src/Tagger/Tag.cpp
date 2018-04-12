#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Tag.h"


Tag::Tag()
{

    tgtpe=new TagType();
    tagStart = -1;
    tagLength = -1;
}

Tag::Tag(TagType *tgtype, int s, int l)
{  tgtpe= tgtype;
    tagStart = s;
     tagLength = l;
     
}

void Tag::SetStart(int s)
{
    tagStart = s;
}

void Tag::SetLength(int l)
{
    tagLength = l;
}

void Tag::SetTagType(TagType *tgtyp)
{
    tgtpe = tgtyp;
}

int Tag::GetStart()
{
    return tagStart;
}

int Tag::GetLength()
{
    return tagLength;
}

TagType* Tag::GetTagType()
{
    return tgtpe;
}
bool Tag::operator==(Tag &t2)
{
    if ((t2.tagLength==tagLength) &&(t2.tagStart==tagStart) &&(t2.tgtpe==tgtpe))
        return 1;
    else
        return 0;
}
bool Tag::operator==(const Tag &t2)
{
    if ((t2.tagLength==tagLength) &&(t2.tagStart==tagStart) &&(t2.tgtpe==tgtpe))
        return 1;
    else
        return 0;
}
/*Tag Tag::operator=(Tag &t1)
{

Tag t2(t1.GetTagType(),t1.GetStart(),t1.GetLength());
return t2;
}
*/
