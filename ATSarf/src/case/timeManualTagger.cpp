#include "timeManualTagger.h"
#include <QtAlgorithms>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QList>
#include <QPair>
#include "logger.h"
#include "letters.h"

TimeTaggerDialog::TimeTaggerDialog(QString filename, QString ext):QMainWindow() {

	this->filename=filename;
	this->ext=ext;
	text=new QTextBrowser(this);
	tag=new QPushButton("&Tag",this);
	unTag=new QPushButton("&Un-Tag",this);
	save=new QPushButton("&Save",this);
	scrollArea=new QScrollArea(this);
	grid=new QGridLayout(scrollArea);
	grid->addWidget(tag,0,0);
	grid->addWidget(unTag,0,1);
	grid->addWidget(save,0,2);
	grid->addWidget(text,1,0,1,3);
	setCentralWidget(scrollArea);
#ifdef ERRORS_TIME
	errors=new QTextBrowser(this);
	errors->resize(errors->width(),50);
	errors_text=new QString();
	grid->addWidget(errors,2,0,1,3);
	displayed_error.setString(errors_text);
	errors->setText(*errors_text);
#endif
	connect(tag,SIGNAL(clicked()),this,SLOT(tag_clicked()));
	connect(unTag,SIGNAL(clicked()),this,SLOT(unTag_clicked()));
	connect(save,SIGNAL(clicked()),this,SLOT(save_clicked()));
	open_action();
	setWindowTitle(filename);
	this->resize(700,700);
}

TimeTaggerDialog::~TimeTaggerDialog() {
	delete tag;
	delete unTag;
	delete save;
	delete text;
	delete scrollArea;
#ifdef ERRORS_TIME
	delete errors;
	delete errors_text;
#endif
	delete grid;
}

void TimeTaggerDialog::tag_action() {
	if (this==NULL)
		return;
	//moveSelectionToWordBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
	Selection * sel=NULL;
	while (i>=0) {
		if (sel==NULL) {
			sel=&tags[i];
		} else {
			sel->first=min(sel->first,tags[i].first);
			sel->second=max(sel->second,tags[i].second);
			tags.removeAt(i);
			i--;
		}
		i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
	}
	if (sel==NULL) {
		tags.append(Selection(start,end-1));
	} else if (i>=0) { //stopped before completion
		start=sel->first;
		end=sel->second;
	} else {
		sel->first=min(sel->first,start);
		sel->second=max(sel->second,end-1);
	}
	c.setPosition(start,QTextCursor::MoveAnchor);
	c.setPosition(end,QTextCursor::KeepAnchor);
	text->setTextCursor(c);
	text->setTextBackgroundColor(Qt::darkYellow);
	c.clearSelection();
	text->setTextCursor(c);
}

void TimeTaggerDialog::unTag_action() {
	if (this==NULL)
		return;
	SelectionList listForRemoval;
	int i=findSelection(0,SELECTION_OUTSIDE);
	if (i>=0) {
		while (i>=0) {
			listForRemoval.append(Selection(tags[i].first,tags[i].second));
			tags.removeAt(i);
			i--;
			i=findSelection(i+1,SELECTION_OUTSIDE);
		}
	} else {
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
		if (i>=0) {
			listForRemoval.append(Selection(start,end-1));
		}
		while (i>=0) {
			if (tags[i].first>=start) {
				tags[i].first=end;
			} else {
				tags[i].second=start-1;
			}
			if (tags[i].first==tags[i].second) {
				tags.removeAt(i);
				i--;
			}
			i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
		}
	}
	QTextCursor c=text->textCursor();
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second+1,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
}

void TimeTaggerDialog::save_action() {
#ifdef SUBTRACT_FACTOR
	int factor=115149;
	for (int i=0;i<tags.size();i++) {
		Selection & s=tags[i];
		s.first-=factor;
		s.second-=factor;
	}
#endif
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1"+ext).arg(filename).toStdString().data());
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

		QFile file(QString("%1"+ext).arg(filename).toStdString().data());
		if (file.open(QIODevice::ReadOnly))
		{
			QDataStream out(&file);   // we will serialize the data into the file
			out	>> tags;
			file.close();
			for (int i=0;i<tags.size();i++) {
				int start=tags[i].first;
				int end=tags[i].second;
				QString all=text->toPlainText();
				if (isDelimiter(all.at(end))) {
					end--;
					tags[i].second=end;
				}
				QTextCursor c=text->textCursor();
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(end+1,QTextCursor::KeepAnchor);
				text->setTextCursor(c);
				text->setTextBackgroundColor(Qt::darkYellow);
			}
			c.clearSelection();
			c.setPosition(0);
			text->setTextCursor(c);

		} else {
			error << "Annotation File does not exist\n";
		}
	} else {
		error << "File does not exist\n";
	}
}



int timeTagger(QString input_str/*, QString extension=".tag"*/){
	TimeTaggerDialog * d=new TimeTaggerDialog(input_str);
	d->show();
	return 0;
}
int hadith_name_annotation(QString input_str,ATMProgressIFC *) {
	TimeTaggerDialog * d=new TimeTaggerDialog(input_str,".names");
	d->show();
	return 0;
}

int narrator_annotation(QString input_str,ATMProgressIFC *) {
	TimeTaggerDialog * d=new TimeTaggerDialog(input_str,".narr");
	d->show();
	return 0;
}

