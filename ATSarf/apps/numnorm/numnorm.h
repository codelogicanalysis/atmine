#ifndef H_NUMNORM
#define H_NUMNORM
#include <number.h>
#include <iostream>

class Number;

class NumNorm {
public:
    QString *text;
    int previous;
    int current;
    int currentH;
    bool isKey;
    bool isHundred;
    bool isNumberDone;
    QHash<QString, int> hashGlossInt;
    QVector<int> extractedNumbers;

    NumNorm(QString*);
    void numberFound();
    bool operator()();
};
#endif
