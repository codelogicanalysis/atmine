#ifndef ATMPROGRESSIFC_H
#define ATMPROGRESSIFC_H
#include <QColor>

class ATMProgressIFC
{
public:
	virtual void report(int value)=0;
	virtual void startTaggingText(QString & text)=0;
	virtual void tag(int start, int length,QColor color, bool textcolor=true)=0;
	virtual void finishTaggingText()=0;
};

#endif // ATMPROGRESSIFC_H
