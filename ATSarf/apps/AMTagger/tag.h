#ifndef TAG_H
#define TAG_H
#include <QString>
#include <QVector>
#include "commonS.h"
#include "tagtype.h"

class Tag {
public:
    Tag();
    Tag(const TagType*, int, int, int, Source, int, QString sourceText = "");
//private:
    int id;
    const TagType* tagtype;
    int wordIndex;
    int pos;
    int length;
    Source source;
    QString sourceText;
    bool operator == (const Tag& tag) const;
};

#endif // TAG_H
