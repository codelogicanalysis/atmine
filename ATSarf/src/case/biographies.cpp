#include "biographies.h"

void BiographiesWindow::report(int value)
{
	if (this==NULL)
		return;
	progressBar->setValue(value);
}
void BiographiesWindow::setCurrentAction(const QString & s)
{
	if (this==NULL)
		return;
	progressBar->setFormat(s+"(%p%)");
}

void BiographiesWindow::resetActionDisplay()
{
	if (this==NULL)
		return;
	progressBar->setFormat("%p%");
	progressBar->reset();
}

void BiographiesWindow::tag(int start, int length,QColor color, bool textcolor)
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=text;
	QTextCursor c=taggedBox->textCursor();
	if (length>100) {
		start=start+length-1;
		length=5;
		color=Qt::red;
	}
	c.setPosition(start,QTextCursor::MoveAnchor);
	c.setPosition(start+length,QTextCursor::KeepAnchor);
	taggedBox->setTextCursor(c);
	if (textcolor)
		taggedBox->setTextColor(color);
	else
		taggedBox->setTextBackgroundColor(color);
}

void BiographiesWindow::startTaggingText(QString & text)
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=this->text;
	taggedBox->clear();
	taggedBox->setLayoutDirection(Qt::RightToLeft);
	QTextCursor c=taggedBox->textCursor();
	c.clearSelection();
	c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
	taggedBox->setTextCursor(c);
	taggedBox->setTextBackgroundColor(Qt::white);
	taggedBox->setTextColor(Qt::black);
	taggedBox->setText(text);

}

void BiographiesWindow::finishTaggingText()
{
	if (this==NULL)
		return;
	QTextBrowser * taggedBox=text;
	QTextCursor c=taggedBox->textCursor();
#if 0
	c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
#else
	c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
#endif
	taggedBox->setTextCursor(c);
}

void biographies(NarratorGraph * graph) {
	BiographiesWindow * g=new BiographiesWindow(graph);
	g->show();
}
