#include "word.h"

Word nextWord(QString & text, int pos) {
    Word word;
    word.word = "";
    word.isStatementStart = false;
    if(pos == text.count()) {
        return word;
    }
    int next = pos;
    while((next != text.count()) && !(text.at(next).isLetter())) {
        if(text.at(next) == '.') {
            word.isStatementStart = true;
        }
        next++;
    }

    if(next == text.count()) {
        return word;
    }

    int end = next+1;
    while((end != text.count()) && text.at(end).isLetter()) {
        end++;
    }
    end = end -1;
    word.start = next;
    word.end = end;
    word.word = text.mid(next,end-next+1);
    return word;
}
