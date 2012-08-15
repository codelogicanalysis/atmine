#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>

class POSVerb : public Stemmer
{
private:
    QString text;
    QString RelatedW;

public:

    POSVerb(QString * text);
    bool on_match();
};

#endif // POSVERB_H
