#ifndef HADITHCHAINGRAPH_H
#define HADITHCHAINGRAPH_H

#include <QColor>
#include "narrator_abstraction.h"
#include "abstractGraph.h"


class HadithChainGraph : public AbstractGraph
{
private:
	class NarratorReader: public ATMProgressIFC { //used by getNarrator
	public:
		Narrator * narr;
	public:
		NarratorReader() { narr=NULL;}
		virtual void report(int ){}
		virtual void startTaggingText(QString & ){}
		virtual void tag(int , int ,QColor, bool){}
		virtual void finishTaggingText(){}
		virtual void setCurrentAction(const QString & ){}
		virtual void resetActionDisplay(){}
		virtual QString getFileName(){ return "";}
		virtual void displayGraph(AbstractGraph *) {}
	};
private:
	friend class HadithChainItemModel;
	friend class HadithDagGraph;
	friend class HadithTaggerDialog;
	Chain chain;
public:
	static Narrator * getNarrator(QString & text);
	static Narrator * getNarrator(const Name & name);
	static int read(ChainsContainer & chains, ATMProgressIFC * prg, QString);

public:
    HadithChainGraph();
	HadithChainGraph(Chain & c); //careful pointers are not duplicated
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
	virtual bool isEmpty() const {return chain.m_chain.size()==0;}
};

#endif // HADITHCHAINGRAPH_H
