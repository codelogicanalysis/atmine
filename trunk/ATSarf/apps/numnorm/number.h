#ifndef NUMBER_H
#define NUMBER_H
#include <stemmer.h>
#include <numnorm.h>
#include <word.h>

class NumNorm;

// This enum is used to keep track of the state of the number being nothing, single, or double such as 100 and 200
typedef enum {Nothing, Continue, Done} NumberState;

class Number : public Stemmer
{
private:
    bool isDigitsTens(QStringList& stem_glosses, long& val);
    bool isKey(QStringList& stem_glosses, long& val);
    bool isHundred(QStringList& stem_glosses, long& val);
    NumberState numstate;
public:
    QString * word;
    int start;
    int end;
    long *val;
    QHash<QString, long>* hashGlossInt;
    NumNorm * controller;
    Number(NumNorm *, Word *, long *);
    bool on_match();
};

#endif // NUMBER_H
