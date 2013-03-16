#ifndef TAG_H
#define TAG_H
#include<QString>
#include "commonS.h"

class Tag {
public:
    Tag();
    Tag(QString, int, int, Source);
//private:
    QString type;
    int pos;
    int length;
    Source source;
    bool operator == (const Tag& tag) const;
};

#endif // TAG_H
