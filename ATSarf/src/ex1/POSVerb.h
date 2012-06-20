#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>
#include<QTextStream>

class POSVerb : public Stemmer
{
private:
    QString text;
    QString RelatedW;
    QTextStream & result;

public:

    POSVerb(QString * text, QTextStream & r);
    bool on_match();
};

#endif // POSVERB_H
