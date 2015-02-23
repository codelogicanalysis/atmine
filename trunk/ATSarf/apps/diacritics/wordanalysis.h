#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>

class Solution {
public:
    Solution() {
        length = 0;
        number_of_morphemes = 0;
        prefix_length = 0;
        stem_length = 0;
        suffix_length = 0;
        isValid = true;
    }

    bool isValid;
    int length;
    QVector<QString> prefixPOSs;
    QVector<QString> prefixes;
    QString stemPOS;
    QVector<QString> suffixPOSs;
    QVector<QString> suffixes;
    QString vWord;
    QString vStem;
    int number_of_morphemes;
    int prefix_length;
    int stem_length;
    int suffix_length;
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
