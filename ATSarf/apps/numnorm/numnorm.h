#ifndef NUMNORM_H
#define NUMNORM_H
#include <number.h>
#include <numsolution.h>
#include <iostream>

class Number;

// This enum specifies the type of the number
typedef enum {TenDigit, Hundred, Key, None} NumType;

class NumNorm {
public:
    QString *text;
    long previous;
    long current;
    long currentH;
    bool isKey;
    bool isHundred;
    bool isNumberDone;
    NumType numtype;

    int numberStart;
    int numberEnd;

    QHash<QString, long> hashGlossInt;
    QHash<QString, double> hashFlexible;
    QVector<NumSolution> extractedNumbers;

    explicit NumNorm(QString*);
    void numberFound();
    void digitsTensActions(long val);
    void keyActions(long val);
    void hundredActions(long val);
    bool operator()();
};
#endif
