#include "timeManualTagger.h"
#include <QtAlgorithms>



void TimeTaggerDialog::tag_action() {
	if (this==NULL)
		return;
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	text->setTextBackgroundColor(Qt::darkYellow);
	c.clearSelection();
	text->setTextCursor(c);
	if (end>start)
		tags.append(Selection(start,end));
}

void TimeTaggerDialog::unTag_action() {
	if (this==NULL)
		return;
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	for (int i=0;i<tags.size();i++) {
		if (start<=tags[i].first && end>=tags[i].second) {
			c.setPosition(tags[i].first,QTextCursor::MoveAnchor);
			c.setPosition(tags[i].second,QTextCursor::KeepAnchor);
			text->setTextCursor(c);
			text->setTextBackgroundColor(Qt::white);
			c.clearSelection();
			text->setTextCursor(c);
			tags.removeAt(i);
			i--;
		}
	}
}

void TimeTaggerDialog::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1.tags").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags;
		file.close();
	} else
		error << "Unexpected Error: Unable to open file\n";
}

void TimeTaggerDialog::open_action() {
	QFile fileOriginal(QString(filename).toStdString().data());
	if (fileOriginal.open(QIODevice::ReadOnly))
	{
		text->clear();
		QTextStream s(&fileOriginal);
		s.setCodec("utf-8");
		text->setLayoutDirection(Qt::RightToLeft);
		QTextCursor c=text->textCursor();
		c.clearSelection();
		c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		text->setText(s.readAll());

		QFile file(QString("%1.tags").arg(filename).toStdString().data());
		if (file.open(QIODevice::ReadOnly))
		{
			QDataStream out(&file);   // we will serialize the data into the file
			out	>> tags;
			file.close();
			for (int i=0;i<tags.size();i++) {
				QTextCursor c=text->textCursor();
				int start=tags[i].first;
				int end=tags[i].second;
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(end,QTextCursor::KeepAnchor);
				text->setTextCursor(c);
				text->setTextBackgroundColor(Qt::darkYellow);
			}
			c.clearSelection();
			c.setPosition(0);
			text->setTextCursor(c);

		} else {
			error << "Tag File does not exist\n";
		}
	} else {
		error << "File does not exist\n";
	}
}


int timeTagger(QString input_str){
	TimeTaggerDialog * d=new TimeTaggerDialog(input_str);
	d->show();
	return 0;
}
