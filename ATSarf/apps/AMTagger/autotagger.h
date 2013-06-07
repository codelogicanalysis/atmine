#ifndef AUTOTAGGER_H
#define AUTOTAGGER_H

#include<QSet>
#include "sarftag.h"
#include "word.h"

class AutoTagger
{
public:
    AutoTagger(QString * text, QHash< QString, QSet<QString> > * synSetHash);
    bool operator()();

    QHash< QString, QSet<QString> > *synSetHash;
    QString *text;
};

#endif // AUTOTAGGER_H
