#ifndef SARFTAG_H
#define SARFTAG_H
#include <QHash>
#include "stemmer.h"
#include "ATMProgressIFC.h"
#include "getGloss.h"

class SarfTag : public Stemmer
{
public:
    SarfTag(int start, int length, QString *text, QHash< QString, QHash<QString, QString> > * synSetHash , QWidget *parent=0);
    bool on_match();

private:
    QWidget *parent;
    QString text;
    QString RelatedW;
    int start;
    int length;
    QHash< QString, QHash<QString, QString> > * synSetHash;
};

#endif // SARFTAG_H
