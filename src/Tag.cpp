#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Tag.h"

Tag::Tag(TagType* tgtype, int s, int l)
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

void Tag::SetTagType(TagType* tgtyp)
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
