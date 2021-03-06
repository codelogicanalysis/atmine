#ifndef ATAGGER_H
#define ATAGGER_H

#include <QVector>
#include <QByteArray>
#include <QtAlgorithms>
#include <QMultiHash>
#include "tag.h"
#include "merftag.h"
#include "tagtype.h"
#include "sarftagtype.h"
#include "msformula.h"
#include "nfa.h"
#include "binarym.h"
#include "unarym.h"
#include "sequentialm.h"
#include "keym.h"
#include "commonS.h"
#include "amfiller.h"

class ATagger;

class ATagger {
public:
    ATagger();
    ~ATagger();
    bool insertTag(const TagType*, int, int, int, Source, Dest, int id = -1);
    bool insertTagType(QString, QString, QString, QString, int, bool, bool, bool, Source, Dest);
    bool insertSarfTagType(QString, QVector < Quadruple< QString , QString , QString , QString > > , QString, QString, QString, int, bool, bool, bool, Source, Dest);
    QByteArray dataInJsonFormat(Data _data, QVector<QMultiHash<int,Tag*>* >* filesHash = NULL, QVector<QString>* textFiles = NULL);
    bool buildNFA();
    bool buildActionFile();
    bool runSimulator(bool isBatch = false);
    Match* simulateNFA(NFA* nfa, QString state, int wordIndex);
    void executeActions(NFA* nfa, int index);
    void constructRelations(int index);
    void constructCrossRelations(QString cr);
    void constructUserDefCrossRelations(QString cr);
    bool executeUserCrossRel(Match* entity1, Match* entity2);
    void drawNFA();
    void updateMatch(Match* match,NFA* nfa, QString state, const Tag* tag=NULL);
    QMultiHash<int,Tag*>* tagHash;
    QMultiHash<int,Tag*> compareToTagHash;
    QVector<Match*> simulationVector;
    QVector<RelationM*> crossRelationVector;
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
    QString userCrossRelation;
    bool isSarf;
    bool compareToIsSarf;
    /// This boolean keeps track of whether the current tags are MBF based or MSF based
    bool isTagMBF;
    /// Hash to keep track of word index based on position
    QHash<int,int> wordIndexMap;
    /// Hash to keep track of statement end with a full stop
    QSet<int> isStatementEndFSSet;
    /// Hash to keep track of statement end with a punctuation
    QSet<int> isStatementEndPSet;
    /// Counter to save word count in text
    int wordCount;
    /// Set containing the diacritics
    QSet<QChar> diacriticsSet;
    /// tag uniqued ID
    int uniqueID;
};

#endif // ATAGGER_H
