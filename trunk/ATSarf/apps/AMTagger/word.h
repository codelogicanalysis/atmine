#ifndef WORD_H
#define WORD_H

#include<QString>

class Word {
public:
    QString word;
    int start;
    int end;
    bool isStatementStart;
};

Word nextWord(QString & text, int pos);
#endif // WORD_H
