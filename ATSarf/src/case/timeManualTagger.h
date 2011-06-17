#ifndef TIMEMANUALTAGGER_H
#define TIMEMANUALTAGGER_H

#include "ATMProgressIFC.h"
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QTextBrowser>
#include <QScrollArea>
#include "logger.h"
#include <iostream>
#include <QList>
#include <QPair>

using namespace std;

class TimeTaggerDialog:public QMainWindow{
	Q_OBJECT
public:
	TimeTaggerDialog(QString filename):QMainWindow() {

		this->filename=filename;
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
		//this->resize(700,700);
	}

public slots:
	void tag_clicked() {
		tag_action();
	}
	void unTag_clicked() {
		unTag_action();
	}
	void save_clicked() {
		save_action();
	}

private:
	void tag_action();
	void unTag_action();
	void save_action();
	void open_action();

public:
	typedef QPair<int,int> Selection;
	typedef QList<Selection> SelectionList;
	SelectionList tags;

	QString filename;
	QPushButton * tag, *unTag, *save;
	QTextBrowser * text;
	QScrollArea *scrollArea;
#ifdef ERRORS_TIME
	QTextBrowser * errors;
	QString * errors_text;
#endif
	QGridLayout * grid;

	~TimeTaggerDialog() {
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
};

#endif // TIMEMANUALTAGGER_H
