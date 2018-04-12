#include "word.h"
#include "letters.h"
#include <QSet>

Word nextWord(QString & text, int pos) {

    QSet<QChar> diacriticsSet;
    diacriticsSet.insert(shadde);
    diacriticsSet.insert(shadde);
    diacriticsSet.insert(fatha);
    diacriticsSet.insert(damma);
    diacriticsSet.insert(kasra);
    diacriticsSet.insert(sukun);
    diacriticsSet.insert(kasratayn);
    diacriticsSet.insert(dammatayn);
    diacriticsSet.insert(fathatayn);
    //diacriticsSet.insert(aleft_superscript);
    diacriticsSet.insert(shadde_alef_above);
    diacriticsSet.insert(madda);

    Word word;
    word.word = "";
    word.isStatementStartFS = false;
    word.isStatementStartPunct = false;
    if(pos == text.count()) {
        return word;
    }
    int next = pos;

    while((next != text.count()) && !(text.at(next).isLetter()) && !(text.at(next).isDigit()) && !(diacriticsSet.contains(text.at(next)))) {
        if(text.at(next) == '.') {
            word.isStatementStartFS = true;
        }
        if(text.at(next).isPunct()) {
            word.isStatementStartPunct = true;
        }
        next++;
    }

    if(next == text.count()) {
        return word;
    }

    int end = next+1;
    if(text.at(next).isLetter() || diacriticsSet.contains(text.at(next))) {
        while((end != text.count()) && (text.at(end).isLetter() || diacriticsSet.contains(text.at(end)))) {
            end++;
        }
    }
    else {
        while((end != text.count()) && text.at(end).isDigit()) {
            end++;
        }
    }
    end = end -1;
    word.start = next;
    word.end = end;
    word.word = text.mid(next,end-next+1);
    return word;
}
