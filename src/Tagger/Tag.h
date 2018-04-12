#ifndef TAG_H
#define TAG_H

#include "TagType.h"
class Tag
{public:
Tag();
    Tag(TagType*, int, int);
    void SetStart(int);
    void SetLength(int);
    void SetTagType(TagType*);

    int GetStart();
    int GetLength();
    TagType* GetTagType();
    bool operator==(const Tag &);
    bool operator==(Tag &);

    //Tag operator=(Tag &);

private:
int tagStart;
int tagLength;
TagType* tgtpe;
};

#endif // TAG_H
