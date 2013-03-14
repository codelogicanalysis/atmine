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
    ~ATagger();
    bool insertTag(QString, int, int, Source, Dest);
    //bool insertSarfTag(QString, int, int, Source, Dest);
    bool insertTagType(QString, QString, int, QString, QString, int, bool, bool, bool, Source, Dest);
    bool insertSarfTagType(QString, QVector < Quadruple< QString , QString , QString , QString > > , QString, int, QString, QString, int, bool, bool, bool, Source, Dest);
    QByteArray dataInJsonFormat(Data _data);
//private:
    QVector<Tag> tagVector;
    QVector<Tag> compareToTagVector;
    QVector<TagType*> *tagTypeVector;
    QVector<TagType*> *compareToTagTypeVector;
    //QVector<SarfTagType> *sarfTagTypeVector;
    QString text;
    QString textFile;
    QString tagFile;
    QString compareToTagFile;
    //QString sarftagFile;
    QString tagtypeFile;
    QString compareToTagTypeFile;
    //QString sarftagtypeFile;
    bool isSarf;
    bool compareToIsSarf;
};

#endif // ATAGGER_H
