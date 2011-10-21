#ifndef TWOLEVELTAGGERSELECTION_H
#define TWOLEVELTAGGERSELECTION_H

#include <QPair>
#include <QList>
#include "abstractGraph.h"


class QString;
class Name;

class Selection {
	friend class AbstractTwoLevelAnnotator;
	friend QDataStream &operator<<(QDataStream &out, const Selection &t);
	friend QDataStream &operator>>(QDataStream &in, Selection &t);
public:
	typedef AbstractGraph::MainSelection MainSelection;
	typedef AbstractGraph::MainSelectionList MainSelectionList;
private:
	MainSelection main;
	MainSelectionList names;
	AbstractGraph * graph;
	QString * text;

private:
	virtual void fillNullGraph() {
		graph->fillNullGraph();
	}
	virtual void addNameToGraph(Name & name){
		graph->addNameToGraph(name);
	}
	virtual void removeNameFromGraph(Name & name){
		graph->removeNameFromGraph(name);
	}
	int getNameIndex(QString s);
	void removeExtraNames();

public:
	Selection();
	Selection(QString * text,int start,int end);
	virtual void addName(int start,int end);
	virtual void addName( const Name & name);
	int getMainStart() const { return main.first;}
	int getMainEnd() const {return main.second;}
	bool operator <(const Selection & second) const {return main<second.main;}
	virtual AbstractGraph * getGraph() { return graph;}
	const MainSelectionList & getNamesList() { return names;}
	virtual void removeNameAt(int i);
	virtual bool updateGraph(QString text){
		return graph->buildFromText(text,this,NULL,NULL);
	}
	virtual QString getText(){
		return graph->getText();
	}
	void setMainInterval(int start,int end);
	virtual void setGraph(AbstractGraph * graph);
	virtual void setText(QString *text);
	virtual void clear();
	static int mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames);
};

inline QDataStream &operator<<(QDataStream &out, const Selection &t) {
	out<<t.main<<t.names;
	t.graph->writeToStream(out);
	return out;
}

inline QDataStream &operator>>(QDataStream &in, Selection &t) {
	in>>t.main >> t.names;
	t.graph->readFromStream(in);
	return in;
}


#endif // TWOLEVELTAGGERSELECTION_H
