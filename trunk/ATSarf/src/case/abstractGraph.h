#ifndef ABSTRACTGRAPH_H
#define ABSTRACTGRAPH_H

#include <QPair>
#include <QList>
#include "ATMProgressIFC.h"

class QString;
class QAbstractItemModel;
class Name;
class TwoLevelSelection;
class AbstractTwoLevelAnnotator;

class AbstractGraph {
public:
	typedef QPair<int,int> MainSelection;
	typedef QList<MainSelection> MainSelectionList;
private:
	virtual AbstractGraph * readFromStreamHelper(QDataStream & in)=0;
public:
	AbstractGraph * newGraph();
	virtual AbstractGraph * duplicate()=0;
	virtual AbstractGraph * merge(AbstractGraph * graph2)=0;
	virtual bool isRepresentativeOf(const MainSelectionList & list)=0;
	virtual QString getText()=0;
	virtual bool buildFromText(QString text,TwoLevelSelection * sel,QString * string=NULL,AbstractTwoLevelAnnotator * annotator=NULL)=0;
	virtual QAbstractItemModel * getTreeModel()=0;
	virtual void displayGraph(ATMProgressIFC * prg)=0; //returns the fileName
	virtual void deleteGraph()=0;
	virtual void fillTextPointers(QString * text)=0;
	AbstractGraph * readFromStream(QDataStream & in){
		if (this==NULL) {
			AbstractGraph * g=newGraph();
			return g->readFromStreamHelper(in);
		} else {
			return readFromStreamHelper(in);
		}
	}
	virtual void writeToStream(QDataStream & out)=0; //if this==NULL, allocate a new object
	virtual void fillNullGraph(MainSelectionList & names, QString * text)=0;
	virtual void addNameToGraph(Name & name)=0;
	virtual void removeNameFromGraph(Name & name)=0;
};

#endif // ABSTRACTGRAPH_H
