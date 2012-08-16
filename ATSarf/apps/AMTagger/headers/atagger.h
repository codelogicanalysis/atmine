#ifndef ATAGGER_H
#define ATAGGER_H

#include<QVector>
#include "tag.h"
#include "tagtype.h"

class ATagger {
public:
    ATagger();
    bool insertTag(QString, int, int, Source);
    bool insertTagType(QString, QString, int, QString, QString, int, bool, bool, bool);
//private:
    QVector<Tag> *tagVector;
    QVector<TagType> *tagTypeVector;
    QString textFile;
    QString tagFile;
    QString tagtypeFile;
};

#endif // ATAGGER_H
