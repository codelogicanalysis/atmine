#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>

class WordAmbiguity : public Stemmer
{
private:
    QString *text;
    int *count;
public:

    WordAmbiguity(QString *text, int *count);
    bool on_match();
    int getAmbiguity();
};

#endif // POSVERB_H
