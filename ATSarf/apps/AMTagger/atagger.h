#ifndef ATAGGER_H
#define ATAGGER_H

#include <QVector>
#include <QByteArray>
#include "tag.h"
#include "tagtype.h"
#include "commonS.h"

class ATagger;

class ATagger {
public:
    ATagger();
    bool insertTag(QString, int, int, Source);
    bool insertTagType(QString, QString, int, QString, QString, int, bool, bool, bool);
    QByteArray dataInJsonFormat(Data _data);
//private:
    QVector<Tag> *tagVector;
    QVector<TagType> *tagTypeVector;
    QString text;
    QString textFile;
    QString tagFile;
    QString tagtypeFile;
};

#endif // ATAGGER_H
