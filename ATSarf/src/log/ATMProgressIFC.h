#ifndef ATMPROGRESSIFC_H
#define ATMPROGRESSIFC_H

class QColor;
class QString;
class AbstractGraph;

class ATMProgressIFC {
    public:
        virtual void report(int value) = 0;
        virtual void startTaggingText(QString &text) = 0;
        virtual void tag(int start, int length, QColor color, bool textcolor = true) = 0;
        virtual void finishTaggingText() = 0;
        virtual void setCurrentAction(const QString &s) = 0;
        virtual void resetActionDisplay() = 0;
        virtual QString getFileName() = 0;
        virtual void displayGraph(AbstractGraph * /*graph*/) {}
};

class EmptyProgressIFC: public ATMProgressIFC {
    public:
        virtual void report(int value);
        virtual void startTaggingText(QString &text);
        virtual void tag(int start, int length, QColor color, bool textcolor = true);
        virtual void finishTaggingText();
        virtual void setCurrentAction(const QString &s);
        virtual void resetActionDisplay();
        virtual QString getFileName();
};



#endif // ATMPROGRESSIFC_H
