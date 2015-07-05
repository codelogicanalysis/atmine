#include "abstractAnnotator.h"
#include "letters.h"

int AbstractAnnotator::findSelection(int startIndex, SelectionMode selectionMode) { //based on user text selection
    QTextBrowser * text=getTextBrowser();
    QTextCursor c=text->textCursor();
    int start=c.selectionStart();
    int end=c.selectionEnd()-1;
    for (int i=startIndex;i<getTagCount();i++) {
        if (isConsistentWithSelectionCondidtion(start,end,getTagStart(i),getTagEnd(i),selectionMode)) {
            return i;
        }
    }
    return -1;
}
void AbstractAnnotator::moveSelectionToWordBoundaries() {
    QTextBrowser * text=getTextBrowser();
    QTextCursor c=text->textCursor();
    int start=c.selectionStart();
    int end=c.selectionEnd();
    QChar chr=(c.selectedText().length()>0?c.selectedText().at(0):'\0');
    if (isDelimiterOrNumber(chr)) {
        while (isDelimiterOrNumber(chr)) {
            c.setPosition(++start,QTextCursor::MoveAnchor);
            c.setPosition(end,QTextCursor::KeepAnchor);
            if (start==end)
                return;
            chr=c.selectedText().at(0);
        }
    } else {
        if (start>0) {
            while (!isDelimiterOrNumber(chr)) {
                c.setPosition(--start,QTextCursor::MoveAnchor);
                c.setPosition(end,QTextCursor::KeepAnchor);
                if (start==0) {
                    start--;
                    break;
                }
                chr=c.selectedText().at(0);
            }
            start++;
        }
    }
    chr=(c.selectedText()>0?c.selectedText().at(c.selectedText().length()-1):'\0');
    if (isDelimiterOrNumber(chr)) {
        while (isDelimiterOrNumber(chr)) {
            c.setPosition(start,QTextCursor::MoveAnchor);
            c.setPosition(--end,QTextCursor::KeepAnchor);
            if (c.selectedText().length()==0)
                return;
            chr=c.selectedText().at(c.selectedText().length()-1);
        }
    } else {
        if (text->toPlainText().length()>end) {
            while (!isDelimiterOrNumber(chr)) {
                c.setPosition(start,QTextCursor::MoveAnchor);
                c.setPosition(++end,QTextCursor::KeepAnchor);
                if (text->toPlainText().length()==end) {
                    end++;
                    break;
                }
                chr=c.selectedText().at(c.selectedText().length()-1);
            }
            end--;
        }
    }
    c.setPosition(start,QTextCursor::MoveAnchor);
    c.setPosition(end,QTextCursor::KeepAnchor);
    text->setTextCursor(c);
}
void AbstractAnnotator::moveSelectionToSentenceBoundaries() {
    QTextBrowser * text=getTextBrowser();
    moveSelectionToWordBoundaries();
    QTextCursor c=text->textCursor();
    int start=c.selectionStart();
    int end=c.selectionEnd();
    if (start==end)
        return;
    QChar chr=c.selectedText().at(c.selectedText().length()-1);
    assert (!isPunctuationMark(chr));
    if (text->toPlainText().length()>end) {
        while (!isPunctuationMark(chr)) {
            c.setPosition(start,QTextCursor::MoveAnchor);
            c.setPosition(++end,QTextCursor::KeepAnchor);
            chr=c.selectedText().at(c.selectedText().length()-1);
            if (text->toPlainText().length()==end) {
                break;
            }
        }
    }
    chr=c.selectedText().at(0);
    assert (!isPunctuationMark(chr));
    if (start>0) {
        while (!isPunctuationMark(chr)) {
            c.setPosition(--start,QTextCursor::MoveAnchor);
            c.setPosition(end,QTextCursor::KeepAnchor);
            chr=c.selectedText().at(0);
            if (start==0) {
                break;
            }
        }
    }
    while (isDelimiter(chr)) {
        c.setPosition(++start,QTextCursor::MoveAnchor);
        c.setPosition(end,QTextCursor::KeepAnchor);
        chr=c.selectedText().at(0);
    }
    c.setPosition(start,QTextCursor::MoveAnchor);
    c.setPosition(end,QTextCursor::KeepAnchor);
    text->setTextCursor(c);
}
