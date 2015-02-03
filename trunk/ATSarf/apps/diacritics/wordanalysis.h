#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>

struct Solution {
    bool isValid;
    int length;
    QString stemPOS;
    QString stem;
};

class WordAnalysis : public Stemmer
{
private:
    QString *text;
    int *count;
public:

    WordAnalysis(QString *text, int *count);
    bool on_match();
    int getAmbiguity();
    QVector<Solution> solutions;
};

#endif // POSVERB_H
