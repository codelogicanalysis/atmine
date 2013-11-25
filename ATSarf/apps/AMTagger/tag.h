#ifndef TAG_H
#define TAG_H
#include <QString>
#include <QVector>
#include "commonS.h"
#include "tagtype.h"

class Tag {
public:
    Tag();
    Tag(const TagType*, int, int, int, Source);
//private:
    const TagType* tagtype;
    int wordIndex;
    int pos;
    int length;
    Source source;
    bool operator == (const Tag& tag) const;
    /// This variable will be used in MERF tags only to know the match type:
    /// STARF, STARL, PLUSF, PLUSL, UPTOF, UPTOL, QUESTIONF, QUESTIONL, ORF, ORL, ANDF, ANDL, SEQF, SEQL
    QVector<QString> tType;
};

#endif // TAG_H
