#ifndef ATAGGER_H
#define ATAGGER_H

#include <QVector>
#include <QByteArray>
#include "tag.h"
#include "merftag.h"
#include "tagtype.h"
#include "sarftagtype.h"
#include "msformula.h"
#include "nfa.h"
#include "commonS.h"

class ATagger;

class ATagger {
public:
    ATagger();
    ~ATagger();
    bool insertTag(QString, int, int, Source, Dest);
    bool insertTagType(QString, QString, int, QString, QString, int, bool, bool, bool, Source, Dest);
    bool insertSarfTagType(QString, QVector < Quadruple< QString , QString , QString , QString > > , QString, int, QString, QString, int, bool, bool, bool, Source, Dest);
    QByteArray dataInJsonFormat(Data _data);
    bool buildNFA();
    bool runSimulator();
    QVector<Tag*>* simulateNFA(NFA* nfa, QString state, int tagIndex);
    QVector<Tag> tagVector;
    QVector<Tag> compareToTagVector;
    QVector<MERFTag> simulationVector;
    QVector<TagType*> *tagTypeVector;
    QVector<TagType*> *compareToTagTypeVector;
    QVector<MSFormula*> *msfVector;
    QVector<MSFormula*> *tempMSFVector;
    QVector<NFA*> *nfaVector;
    QString text;
    QString textFile;
    QString tagFile;
    QString compareToTagFile;
    QString tagtypeFile;
    QString compareToTagTypeFile;
    bool isSarf;
    bool compareToIsSarf;
};

#endif // ATAGGER_H
