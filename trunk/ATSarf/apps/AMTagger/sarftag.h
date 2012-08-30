#ifndef SARFTAG_H
#define SARFTAG_H
#include "stemmer.h"
#include "ATMProgressIFC.h"

class SarfTag : public Stemmer
{
public:
    SarfTag(int start, int length, QString *text, QWidget *parent=0);
    bool on_match();

private:
    QWidget *parent;
    QString text;
    QString RelatedW;
    int start;
    int length;
};

#endif // SARFTAG_H
