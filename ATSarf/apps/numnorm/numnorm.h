#ifndef H_NUMNORM
#define H_NUMNORM
#include <number.h>
#include <numsolution.h>
#include <iostream>

class Number;

// This enum specifies the type of the number
typedef enum {TenDigit, Hundred, Key, None} NumType;

class NumNorm {
public:
    QString *text;
    int previous;
    int current;
    int currentH;
    bool isKey;
    bool isHundred;
    bool isNumberDone;
    NumType numtype;

    int numberStart;
    int numberEnd;

    QHash<QString, int> hashGlossInt;
    QHash<QString, int> hashFlexible;
    QVector<NumSolution> extractedNumbers;

    NumNorm(QString*);
    void numberFound();
    void digitsTensActions(int val);
    void keyActions(int val);
    void hundredActions(int val);
    bool operator()();
};
#endif
