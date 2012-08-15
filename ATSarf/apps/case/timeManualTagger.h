#ifndef TIMEMANUALTAGGER_H
#define TIMEMANUALTAGGER_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextBrowser>
#include <QScrollArea>
#include <QGridLayout>


#include "ATMProgressIFC.h"
#include "abstractAnnotator.h"

//#define SUBTRACT_FACTOR

using namespace std;

class TimeTaggerDialog:public QMainWindow,public AbstractAnnotator{
	Q_OBJECT
public:
	TimeTaggerDialog(QString filename, QString ext=".tags");
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
	QTextBrowser * getTextBrowser() {return text;}
	int getTagCount() const{ return tags.size();}
	int getTagStart(int i) const{return tags.at(i).first;}
	int getTagEnd(int i) const{return tags.at(i).second;}
public:
	typedef QPair<int,int> Selection;
	typedef QList<Selection> SelectionList;
	SelectionList tags;

	QString filename,ext;
	QPushButton * tag, *unTag, *save;
	QTextBrowser * text;
	QScrollArea *scrollArea;
#ifdef ERRORS_TIME
	QTextBrowser * errors;
	QString * errors_text;
#endif
	QGridLayout * grid;

	~TimeTaggerDialog();
};

#endif // TIMEMANUALTAGGER_H
