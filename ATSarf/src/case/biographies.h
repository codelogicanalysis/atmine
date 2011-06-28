#ifndef BIOGRAPHIES_H
#define BIOGRAPHIES_H

#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QHeaderView>
#include <QStringList>
#include <QComboBox>
#include <QTableWidget>
#include <QTextBrowser>
#include <QInputDialog>

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>

#include "sql_queries.h"
#include "logger.h"
#include "ATMProgressIFC.h"
#include "graph.h"
#include <iostream>

using namespace std;

#define ERRORS_BIO

class BiographiesWindow:public QMainWindow,public ATMProgressIFC{
	Q_OBJECT
public:
	BiographiesWindow(NarratorGraph * graph):QMainWindow() {
		this->graph=graph;
		text=new QTextBrowser(this);
		parse=new QPushButton("&Parse Biographies",this);
		color=new QPushButton("&Color Biography",this);
		browse=new QPushButton("&Browse",this);
		input=new QTextEdit(this);
		biographyNum=new QComboBox(this);
		scrollArea=new QScrollArea(this);
		subScrollArea=new QScrollArea(scrollArea);
		pic=new QLabel(subScrollArea);
		subScrollArea->setWidget(pic);
		progressBar=new QProgressBar(this);
		grid=new QGridLayout(scrollArea);
		grid->addWidget(input,0,0,1,3);
		grid->addWidget(browse,0,3);
		grid->addWidget(parse,0,4);
		grid->addWidget(biographyNum,0,5);
		grid->addWidget(color,0,6);
		grid->addWidget(progressBar,0,7,1,3);
		grid->addWidget(text,1,0,3,5);
		grid->addWidget(subScrollArea,1,5,3,5);
		grid->setRowMinimumHeight(0,50);
		grid->setRowStretch(0,0);
		grid->setRowStretch(1,150);
		grid->setRowMinimumHeight(1,500);
		grid->setRowMinimumHeight(2,300);
		grid->setRowStretch(2,0);
		grid->setColumnStretch(0,150);
		grid->setColumnStretch(7,150);
		subScrollArea->setWidget(pic);
		setCentralWidget(scrollArea);
		subScrollArea->setWidgetResizable(true);
		connect(parse,SIGNAL(clicked()),this,SLOT(parse_clicked()));
		connect(color,SIGNAL(clicked()),this,SLOT(color_clicked()));
		connect(browse,SIGNAL(clicked()),this,SLOT(browse_clicked()));
		setWindowTitle("Biographies");
		this->resize(900,700);
		browseFileDlg=NULL;
		biographyList=NULL;
	#ifdef ERRORS_BIO
		errors=new QTextBrowser(this);
		errors->resize(errors->width(),50);
		errors_text=new QString();
		grid->addWidget(errors,4,0,1,10);
		displayed_error.setString(errors_text);
		out.setString(errors_text);
	#endif
		displayUncoloredGraph();
	#ifdef ERRORS_BIO
		errors->setText(*errors_text);
	#endif
	}

public slots:
	void parse_clicked() {
		QString fileName=input->toPlainText();
		biographyList=getBiographies(fileName,this);
		biographyNum->clear();
		for (int i=0;i<biographyList->size();i++) {
			for (int j=0;j<biographyList->at(i)->size();j++) {
				Narrator * n=(*biographyList->at(i))[j];
				ChainNarratorNode * c=graph->getNodeMatching(*n);
				if (c!=NULL)
					c->addBiographyIndex(i);
			}
			biographyNum->addItem(QString("%1").arg(i));
		}
		displayUncoloredGraph();
	}
	void color_clicked() {
		int num=biographyNum->currentText().toInt();
		setCurrentAction("Display Graph");
		report(0);
		DisplayNodeVisitorColored visitor(num);
		GraphVisitorController c(&visitor,graph);
		graph->DFS_traverse(c);
		setCurrentAction("Completed");
		report(100);
		try{
			system("dot -Tsvg graph.dot -o graph.svg");
			pic->setPixmap(QPixmap("./graph.svg"));
			subScrollArea->setWidget(pic);
		}catch(...) {}

	}
	void browse_clicked() {
		QString fileName=getFileName(browseFileDlg);
		if (!fileName.isEmpty())
			input->setText(fileName);
	}

private:
	virtual void report(int value);
	virtual void tag(int start, int length,QColor color, bool textcolor);
	virtual void startTaggingText(QString & text);
	virtual void finishTaggingText();
	virtual void setCurrentAction(const QString & s);
	virtual void resetActionDisplay();

	void displayUncoloredGraph(){
		DisplayNodeVisitor visitor;
		GraphVisitorController c(&visitor,graph);
		graph->DFS_traverse(c);
		try{
			system("dot -Tsvg graph.dot -o graph.svg");
			pic->setPixmap(QPixmap("./graph.svg"));
			subScrollArea->setWidget(pic);
		}catch(...) {}
	}

public:

	QPushButton * parse, *color, *browse;
	QTextBrowser * text;
	QScrollArea *scrollArea,* subScrollArea;
	QTextEdit *input;
	QComboBox * biographyNum;
	QLabel * pic;
	QProgressBar *progressBar;
#ifdef ERRORS_BIO
	QTextBrowser * errors;
	QString * errors_text;
#endif
	QGridLayout * grid;
	QFileDialog * browseFileDlg;

	NarratorGraph * graph;
	BiographyList * biographyList;

	~BiographiesWindow() {
		delete parse;
		delete color;
		delete browse;
		delete text;
		delete input;
		delete biographyNum;
		delete scrollArea;
		delete subScrollArea;
		delete pic;
		delete progressBar;
	#ifdef ERRORS_BIO
		delete errors;
		delete errors_text;
	#endif
		delete grid;

		delete graph;
		if (biographyList!=NULL)
			delete biographyList;
		if (browseFileDlg!=NULL)
			delete browseFileDlg;
	}
};
#endif // BIOGRAPHIES_H
