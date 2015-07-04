#ifndef SARFTAG_H
#define SARFTAG_H
#include <QHash>
#include <QSet>
#include "stemmer.h"
#include "ATMProgressIFC.h"
#include "getGloss.h"

class SarfTag : public Stemmer
{
public:
    SarfTag(int start,
            int length,
            QString *text,
            QHash< QString, QSet<QString> > * synSetHash ,
            QSet<QString>* eNF,
            QHash<QString, QString>* iNF,
            QWidget *parent=0);
    bool on_match();

private:
    QString text;
    QString RelatedW;
    int start;
    int length;
    QHash< QString, QSet<QString> > * synSetHash;
    QSet<QString>* eNF;
    QHash<QString, QString>* iNF;
};

#endif // SARFTAG_H
