#ifndef TAG_H
#define TAG_H

#include "TagType.h"
class Tag
{public:

    Tag(TagType *, int, int);
    void SetStart(int);
    void SetLength(int);
    void SetTagType(TagType*);

    int GetStart();
    int GetLength();
    TagType* GetTagType();


int tagStart;
int tagLength;
TagType* tgtpe;
};

#endif // TAG_H
