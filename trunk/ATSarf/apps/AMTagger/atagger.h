#ifndef ATAGGER_H
#define ATAGGER_H

#include <QVector>
#include <QByteArray>
#include "tag.h"
#include "tagtype.h"
#include "sarftagtype.h"
#include "commonS.h"

class ATagger;

class ATagger {
public:
    ATagger();
    bool insertTag(QString, int, int, Source);
    bool insertSarfTag(QString, int, int, Source);
    bool insertTagType(QString, QString, int, QString, QString, int, bool, bool, bool, Source);
    bool insertSarfTagType(QString, QVector < QPair <QString , QString> > , QString, int, QString, QString, int, bool, bool, bool, Source);
    QByteArray dataInJsonFormat(Data _data);
//private:
    QVector<Tag> *tagVector;
    //QVector<Tag> *sarfTagVector;
    QVector<TagType*> *tagTypeVector;
    //QVector<SarfTagType> *sarfTagTypeVector;
    QString text;
    QString textFile;
    QString tagFile;
    //QString sarftagFile;
    QString tagtypeFile;
    QString sarftagtypeFile;
    bool isSarf;
};

#endif // ATAGGER_H
