#ifndef TWOLEVELTAGGERSELECTION_H
#define TWOLEVELTAGGERSELECTION_H

#include <QPair>
#include <QList>
#include "abstractGraph.h"


class QString;
class Name;

class TwoLevelSelection {
    friend class AbstractTwoLevelAnnotator;
    friend QDataStream &operator<<(QDataStream &out, const TwoLevelSelection &t);
    friend QDataStream &operator>>(QDataStream &in, TwoLevelSelection &t);
    friend class GeneTree;
    friend class HadithChainGraph;
    friend class OneLevelAgreement;
    public:
    typedef AbstractGraph::MainSelection MainSelection;
    typedef AbstractGraph::MainSelectionList MainSelectionList;

    private:
    MainSelection main;
    MainSelectionList names;
    AbstractGraph * graph;
    QString * text;

    private:
    virtual void fillNullGraph(MainSelectionList & names) {
        if (graph->isEmpty() && names.size()>0) {
            graph->fillNullGraph(names,text);
        }
    }
    virtual void addNameToGraph(Name & name){
        graph->addNameToGraph(name);
    }
    virtual void removeNameFromGraph(Name & name){
        graph->removeNameFromGraph(name);
    }
    void removeExtraNames();

    public:
    TwoLevelSelection(AbstractGraph * graph);
    TwoLevelSelection(AbstractGraph * graph,QString * text,int start,int end);
    virtual ~TwoLevelSelection() {}
    virtual void addName(int start,int end);
    virtual void addName( const Name & name);
    int getMainStart() const { return main.first;}
    int getMainEnd() const {return main.second;}
    bool operator <(const TwoLevelSelection & second) const {return main<second.main;}
    virtual AbstractGraph * getGraph() { return graph;}
    const MainSelectionList & getNamesList() { return names;}
    virtual void removeNameAt(int i);
    virtual bool updateGraph(QString text){
        return graph->buildFromText(text,this,NULL,NULL);
    }
    virtual QString getText(){
        if (this!=NULL && graph!=NULL)
            return graph->getText();
        else
            return "NULL";
    }
    void setMainInterval(int start,int end);
    virtual void setGraph(AbstractGraph * graph);
    virtual void setText(QString *text);
    virtual void clear();
    static int mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames);
    QString * getTextPointer() { return text;}
    int getNameIndex(QString s);
};

inline QDataStream &operator<<(QDataStream &out, const TwoLevelSelection &t) {
    out<<t.main<<t.names;
    t.graph->writeToStream(out);
    return out;
}

inline QDataStream &operator>>(QDataStream &in, TwoLevelSelection &t) {
    in>>t.main >> t.names;
    t.graph=t.graph->readFromStream(in);
    return in;
}


#endif // TWOLEVELTAGGERSELECTION_H
