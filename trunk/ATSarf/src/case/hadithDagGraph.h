#ifndef HADITHDAGGRAPH_H
#define HADITHDAGGRAPH_H

#include "abstractGraph.h"
#include "graph.h"

class ATMProgressIFC;

class HadithDagGraph : public AbstractGraph
{
private:
	class DummyATMProgressIFC:public ATMProgressIFC {
		virtual void report(int ){}
		virtual void startTaggingText(QString & ){}
		virtual void tag(int , int ,QColor , bool ){}
		virtual void finishTaggingText(){}
		virtual void setCurrentAction(const QString & ){}
		virtual void resetActionDisplay(){}
		virtual QString getFileName(){ return "";}
	} dummyATMProgressIFC;
	HadithDagGraph(ChainsContainer & c);
private:
	friend class HadithDagItemModel;
	friend class HadithTaggerDialog;
	NarratorGraph * graph;
public:
	HadithDagGraph();
	virtual AbstractGraph * readFromStreamHelper(QDataStream &in);
	virtual AbstractGraph * duplicate();
	virtual AbstractGraph * merge(AbstractGraph *graph2);
	virtual bool isRepresentativeOf(const MainSelectionList &list);
	virtual QString getText();
	virtual bool buildFromText(QString text, TwoLevelSelection *sel, QString *string, AbstractTwoLevelAnnotator *annotator);
	virtual QAbstractItemModel * getTreeModel();
	virtual void displayGraph(ATMProgressIFC *prg);
	virtual void deleteGraph();
	virtual void fillTextPointers(QString *text);
	virtual void writeToStream(QDataStream &out);
	virtual void fillNullGraph(MainSelectionList &names, QString *text);
	virtual void addNameToGraph(Name &name);
	virtual void removeNameFromGraph(Name &name);
	virtual bool isEmpty() const {return graph->all_nodes.size()==0;}
};

#endif // HADITHDAGGRAPH_H
