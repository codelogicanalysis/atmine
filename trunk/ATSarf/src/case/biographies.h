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
		colorBiography=new QPushButton("Color &Biography",this);
		colorNarrators=new QPushButton("Color &Narrators",this);
		nmc_label=new QLabel("NMC min", this);
		nrc_label=new QLabel("NRC min",this);
		narr_label=new QLabel("NARR min",this);
		nmc_max=new QTextEdit("1",this);
		nmc_max->setAlignment(Qt::AlignCenter);
		nmc_max->setMaximumHeight(30);
		nrc_max=new QTextEdit("100",this);
		nrc_max->setAlignment(Qt::AlignCenter);
		nrc_max->setMaximumHeight(30);
		narr_min=new QTextEdit("4",this);
		narr_min->setAlignment(Qt::AlignCenter);
		narr_min->setMaximumHeight(30);
		browse=new QPushButton("&Browse",this);
		input=new QTextEdit(this);
		input->setMaximumHeight(30);
		biographyNum=new QComboBox(this);
		narratorListDisplay=new QTableWidget(0,2,this);
		narratorListDisplay->clear();
		QStringList v;
		v<<"Narrator"<<"Biography";
		narratorListDisplay->verticalHeader()->setHidden(true);
		narratorListDisplay->setHorizontalHeaderLabels(v);
		narratorListDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
		narratorListDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
		narratorListDisplay->setSelectionMode(QAbstractItemView::MultiSelection);
		scrollArea=new QScrollArea(this);
		subScrollArea=new QScrollArea(scrollArea);
		pic=new QLabel(subScrollArea);
		subScrollArea->setWidget(pic);
		progressBar=new QProgressBar(this);
		grid=new QGridLayout(scrollArea);
		grid->addWidget(input,0,0,1,4);
		grid->addWidget(browse,0,4);
		grid->addWidget(parse,0,5);
		grid->addWidget(progressBar,0,6,1,4);
		grid->addWidget(nmc_label,1,0);
		grid->addWidget(nmc_max,1,1);
		grid->addWidget(nrc_label,1,2);
		grid->addWidget(nrc_max,1,3);
		grid->addWidget(narr_label,1,4);
		grid->addWidget(narr_min,1,5);
		grid->addWidget(biographyNum,1,6);
		grid->addWidget(colorBiography,1,7);
		grid->addWidget(colorNarrators,1,8,1,2);
		grid->addWidget(text,2,0,3,3);
		grid->addWidget(subScrollArea,2,3,3,5);
		grid->addWidget(narratorListDisplay,2,8,3,2);
		grid->setRowMinimumHeight(0,50);
		grid->setRowStretch(0,0);
		grid->setRowStretch(2,150);
		grid->setRowMinimumHeight(2,500);
		grid->setRowMinimumHeight(3,300);
		grid->setRowStretch(3,0);
		//grid->setColumnStretch(0,150);
		//grid->setColumnStretch(7,150);
		subScrollArea->setWidget(pic);
		setCentralWidget(scrollArea);
		subScrollArea->setWidgetResizable(true);
		connect(parse,SIGNAL(clicked()),this,SLOT(parse_clicked()));
		connect(colorBiography,SIGNAL(clicked()),this,SLOT(colorBiography_clicked()));
		connect(colorNarrators,SIGNAL(clicked()),this,SLOT(colorNarrators_clicked()));
		connect(browse,SIGNAL(clicked()),this,SLOT(browse_clicked()));
		setWindowTitle("Biographies");
		this->resize(1100,700);
		browseFileDlg=NULL;
		biographyList=NULL;
	#ifdef ERRORS_BIO
		errors=new QTextBrowser(this);
		errors->resize(errors->width(),50);
		errors_text=new QString();
		grid->addWidget(errors,6,0,1,10);
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
	#ifdef TEST_BIOGRAPHIES
		bool v1,v2,v3;
		hadithParameters.bio_narr_min=narr_min->toPlainText().toInt(&v1);
		hadithParameters.bio_nmc_max=nmc_max->toPlainText().toInt(&v2);
		hadithParameters.bio_nrc_max=nrc_max->toPlainText().toInt(&v3);
		if  (!v1 || !v2 || !v3 ) {
			text->setText("Parameters for Biography Segmentaion are not valid integers!\n");
			return;
		}
		QString fileName=input->toPlainText();
		biographyList=getBiographies(fileName,this);
		if (biographyList==NULL)
			return;
		biographyNum->clear();
		int count=0;
		for (int i=0;i<biographyList->size();i++) {
			for (int j=0;j<biographyList->at(i)->size();j++) {
			#ifndef COLOR_ALL
				Narrator * n=(*biographyList->at(i))[j];
				ChainNarratorNode * c=graph->getNodeMatching(*n);
				if (c!=NULL)
					c->addBiographyIndex(i);
			#else
				Narrator * n=(*biographyList->at(i))[j];
				narratorListDisplay->setRowCount(count+1);
				narratorListDisplay->setItem(count,0,new QTableWidgetItem(n->getString()));
				narratorListDisplay->setItem(count,1,new QTableWidgetItem(QString("%1").arg(i)));
				narratorList.append(n);
				count++;
				ColorBiographiesAction c(i);
				graph->performActionToAllCorrespondingNodes(n,c);
			#endif
			}
			biographyNum->addItem(QString("%1").arg(i));
		}
		displayUncoloredGraph();
	#endif
	}
	void colorBiography_clicked() {
		int num=biographyNum->currentText().toInt();
		setCurrentAction("Display Graph");
		report(0);
		DisplayNodeVisitorColoredBiography visitor(num);
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
	void colorNarrators_clicked() {
		QList<QTableWidgetSelectionRange>  selection=narratorListDisplay->selectedRanges();
		ColorNarratorsAction::DetectedNodesMap map;
		ColorNarratorsAction action(map);
		for (int i=0;i<selection.size();i++) {
			int topRow=selection[i].topRow();
			for (int j=0;j<selection[i].rowCount();j++) {
				int row=topRow+j;
				Narrator * n=narratorList[row];
				graph->performActionToAllCorrespondingNodes(n,action);
			}
		}
		report(0);
		DisplayNodeVisitorColoredNarrator visitor(map);
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

	class ColorBiographiesAction: public NarratorHash::FoundAction {
	private:
		int biographyIndex;
	public:
		ColorBiographiesAction(int biographyIndex) { this->biographyIndex=biographyIndex;}
		virtual void action(const QString &, ChainNarratorNode * node, double ) {
			node->addBiographyIndex(biographyIndex);
		}
	};
	class ColorNarratorsAction: public NarratorHash::FoundAction {
	public:
		typedef QMap<NarratorNodeIfc*,double> DetectedNodesMap;
	private:
		DetectedNodesMap & map;
	public:
		ColorNarratorsAction(DetectedNodesMap & m):map(m) { }
		virtual void action(const QString & s, ChainNarratorNode * node, double v) {
			NarratorNodeIfc * n=&node->getCorrespondingNarratorNode();
			DetectedNodesMap::iterator i = map.find(n);
			#ifdef NARRATORHASH_DEBUG
				qDebug()<<"("<<n->CanonicalName()<<")";
			#endif
			if (i!=map.end()) {
				double oldSimilarity=*i;
				if (oldSimilarity<v) {
					map[n]=v;
				#ifdef NARRATORHASH_DEBUG
					qDebug()<<oldSimilarity<<"--->\t"<<v;
				#endif
				}
			} else {
			#ifdef NARRATORHASH_DEBUG
				qDebug()<<s<<"\t--->\t"<<v;
			#endif
				map[n]=v;
			}
		}
	};

private:

	QPushButton * parse, *colorBiography, *browse, *colorNarrators;
	QLabel * nmc_label, *nrc_label, *narr_label;
	QTextEdit *nmc_max,*nrc_max,*narr_min;
	QTextBrowser * text;
	QScrollArea *scrollArea,* subScrollArea;
	QTextEdit *input;
	QComboBox * biographyNum;
	QLabel * pic;
	QProgressBar *progressBar;
	QTableWidget * narratorListDisplay;
	QList<Narrator *> narratorList;
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
		delete colorBiography;
		delete browse;
		delete text;
		delete input;
		delete biographyNum;
		delete scrollArea;
		delete subScrollArea;
		delete pic;
		delete progressBar;
		delete narratorListDisplay;
		delete colorNarrators;
		delete nmc_label;
		delete nrc_label;
		delete narr_label;
		delete nmc_max;
		delete nrc_max;
		delete narr_min;
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
