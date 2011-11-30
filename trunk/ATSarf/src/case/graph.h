#ifndef GRAPH_H
#define GRAPH_H

#include <QStack>
#include <QInputDialog>
#include <QDir>
#include "narratorHash.h"
#include "graph_nodes.h"
#include "narratordetector.h"

class NarratorGraph;
class GraphVisitorController;
class LoopBreakingVisitor;
typedef QList<NarratorNodeIfc *> NarratorNodesList;

extern void biographies(NarratorGraph * graph);
extern void localizedDisplay(NarratorGraph * graph);
extern int calculateStatisticsOrAnotate(ChainsContainer &chains, NarratorGraph * graph, QString * text, QString fileName);



class NodeVisitor
{
protected:
	GraphVisitorController  * controller;
	friend class GraphVisitorController;
public:
	virtual void initialize()=0;
	virtual bool visit(NarratorNodeIfc & n)=0;
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num)=0;
	virtual void finish()=0;

	virtual void detectedCycle(NarratorNodeIfc & n)=0;
	virtual void finishVisit(NarratorNodeIfc & n)=0;
};

class ColorIndices
{
private:
	NarratorGraph * graph;
	unsigned int usedBits;
	unsigned int nextUnused;

	unsigned int maxBits(){ return sizeof(int)<<3;}
	void setGraph(NarratorGraph * graph){this->graph=graph;}

	friend class NarratorGraph;
public:
	ColorIndices()
	{
		usedBits=0;
		nextUnused=0;
	}
	void use(unsigned int bit)
	{
		assert (bit<maxBits());
		usedBits |= 1 << bit;
		if (nextUnused==bit)
		{
			unsigned int max=maxBits();
			for (unsigned int i=bit+1;i<max;i++)
			{
				if (!isUsed(i))
				{
					nextUnused=i;
					return;
				}
			}
			nextUnused=maxBits();//unacceptable value for usage
		}
	}
	void unUse(unsigned int bit);//unuse and clear color bit for all nodes in graph
	void setAllNodesVisited(unsigned int bit);
	bool isUsed(unsigned int bit)
	{
		assert (bit<maxBits());
		return (usedBits & (1 << bit)) != 0;
	}
	unsigned int getNextUnused()
	{
		return nextUnused;
	}
};

class GraphVisitorController
{
public:
	typedef QStack< NarratorNodeIfc*> ParentStack;
private:
	typedef QPair<NarratorNodeIfc *,NarratorNodeIfc*> Edge;
	typedef QMap<Edge, bool> EdgeMap;
	typedef enum{NOT_VISITED,VISITED,FINISHED} Status;
	typedef QMap<NarratorNodeIfc *, Status>  ColorMap;

	ColorMap * visitedMap;
	bool firstCall:1;
	bool direction:1;
	bool graphBuilt:1;
	bool keep_track_of_edges:1, keep_track_of_nodes:1,merged_edges_as_one:1;

	friend bool GraphNarratorNode::fillChildren();
	friend bool GraphNarratorNode::fillParents();
	bool disableFillChildren:1;

	int unused:26;

	EdgeMap edgeMap;
	ParentStack parentStack;
	unsigned int visitIndex, finishIndex ;

	NarratorGraph * graph;
	NodeVisitor * visitor;

	friend class NarratorGraph;

	void init();
	void construct(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges,bool keep_track_of_nodes, bool merged_edges_as_one);

public:

#ifdef DEFAULT_MERGE_NODES
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,unsigned int visitIndex,unsigned int finishIndex,bool keep_track_of_edges=true, bool merged_edges_as_one=true);
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges=true, bool merged_edges_as_one=true);
#else
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,unsigned int visitIndex,unsigned int finishIndex,bool keep_track_of_edges=false, bool merged_edges_as_one=false);
	GraphVisitorController(NodeVisitor * visitor,NarratorGraph * graph,bool keep_track_of_edges=false, bool merged_edges_as_one=false);
#endif
	bool isPreviouslyVisited( NarratorNodeIfc & node)
	{
		if (!keep_track_of_nodes)
			return false;

		bool visited=false;
		if (graph!=NULL){
			visited= node.isVisited(visitIndex);
		} else {
			ColorMap::iterator i=visitedMap->find(&node);
			visited= (i!=visitedMap->end());
		}
		if (visited) {
			if (!isFinished(node))
				visitor->detectedCycle(node);
		}
		return visited;
	}
	bool isPreviouslyVisited( NarratorNodeIfc & n1, NarratorNodeIfc & n2,int child_num1,int child_num2)
	{
		if (!keep_track_of_edges)
			return false;
		//assert(&n2==&(n1.getChild(child_num1,child_num2))); //to check if the edge exists i.e. we can go from n1 to n2
		if (merged_edges_as_one) {
			//only end-point nodes make a difference
			EdgeMap::iterator it=edgeMap.find(Edge(&n1,&n2));
			return !(it==edgeMap.end());
		}else
			return n1[child_num1][child_num2].isVisited(visitIndex);
	}
	void initialize(int direction=1);
	void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int child_num1,int child_num2)
	{
		visitor->visit(n1,n2,child_num1);
		if (!keep_track_of_edges)
			return;
		if (merged_edges_as_one) {
			//only end point nodes make a difference
			edgeMap.insert(Edge(&n1,&n2),true);
		} else {
			if (child_num1<n1.size()) { //these checks are needed in case this node has been alterred inside visit of the visitor
				GraphNodeItem & g=n1[child_num1];
				if (child_num2<g.size()) {
					g[child_num2].setVisited(visitIndex);
				}
			}
		}
	}
	bool visit(NarratorNodeIfc & n)
	{
		if (keep_track_of_nodes && graph!=NULL) {
			n.resetVisited(finishIndex);
		}
		assert(!n.isGroupNode());
		parentStack.push(&n);
		if (!disableFillChildren && graphBuilt && n.isGraphNode()) {
			GraphNarratorNode * g=dynamic_cast<GraphNarratorNode *>(&n);
			if (direction)
				g->fillChildren();
			else
				g->fillParents();
		}
		bool retVal=visitor->visit(n);

		if (!keep_track_of_nodes)
			return retVal;
		if (graph!=NULL) {
			n.setVisited(visitIndex);
		}
		else {
			(*visitedMap)[&n]=VISITED;
		}
		return retVal;
	}
	bool isFinished( NarratorNodeIfc & node)
	{
		if (!keep_track_of_nodes)
			return false;
		if (graph!=NULL)
			return node.isVisited(finishIndex);
		else {
			ColorMap::iterator i=visitedMap->find(&node);
			assert(i!=visitedMap->end());
			Status s=*i;
			return (s==FINISHED);
		}
	}
	void finishVisit(NarratorNodeIfc & n)
	{
		/*if (keep_track_of_nodes) //TODO: check this
			n.setVisited(finishIndex);*/
		if (!keep_track_of_nodes)
			return;
		if (graph!=NULL)
			n.setVisited(finishIndex);
		else {
			(*visitedMap)[&n]=FINISHED;
		}
		visitor->finishVisit(n);
		assert(&n==parentStack.pop());
	}

	void finish();
	unsigned int getVisitColorIndex(){return visitIndex;}
	unsigned int getFinishColorIndex(){return finishIndex;}
	const ParentStack & getParentStack() {return parentStack;}
	NarratorGraph * getGraph(){return graph;}
	bool isTop2Bottom() {
		return direction;
	}
};

class DisplayNodeVisitor: public NodeVisitor
{
protected:
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
	QList<QStringList> ranksList;

	typedef QMap<NarratorNodeIfc*,int> IDMap;
	IDMap nodeMap;
	int last_id;
	int getUniqueNodeID(NarratorNodeIfc & n)//if not there returns -1
	{
		IDMap::iterator it=nodeMap.find(&n);
		if (it==nodeMap.end())
			return -1;
		else
			return it.value();
	}
	int generateUniqueNodeID(NarratorNodeIfc & n) //if not there generates one and returns it
	{
		int curr_id;
		IDMap::iterator it=nodeMap.find(&n);
		if (it==nodeMap.end()) {
			curr_id=++last_id;
			nodeMap.insert(&n,curr_id);
		}
		else
			curr_id=it.value();
		return curr_id;
	}
	void setGraphRank(int rank, QString s)
	{
		while(rank>=ranksList.size())
			ranksList.append(QStringList());
		ranksList[rank].append(s);
	}
	void displayChainNumsEndingJustAfter(NarratorNodeIfc & n, QString name)
	{
		if (hadithParameters.display_chain_num )
		{
			//qDebug()<<name;
			NodeIterator itr =n.begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode &c=(ChainNarratorNode &)*itr;
				if (c.isLast())
				{
					int num=c.getChainNum();
					QString ch_node=QString("ch%1").arg(num+1);
					d_out<<ch_node<<" [label=\""<<num+1<<"\", shape=triangle];\n";
					d_out<<name<<"->"<<ch_node<<";\n";
					setGraphRank(n.getRank()+2,ch_node);
				}
			}
		}
	}
	virtual QString getOtherAttributes(NarratorNodeIfc & ) {return "";}
	QString getAndInitializeDotNode(NarratorNodeIfc & n)
	{
		int curr_id=getUniqueNodeID(n);
		QString name;
		if (curr_id==-1)
		{
			curr_id=generateUniqueNodeID(n);
			if (n.isGraphNode())
			{
				d_out<<QString("g")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.rank2String();
			#endif
				d_out<<"\",shape=box"<<getOtherAttributes(n)<<"];\n";
				name=QString("g%1").arg(curr_id);
			}
			else
			{
				d_out<<QString("c")<<curr_id<<" [label=\""<<n.CanonicalName().replace('\n',"");
			#ifdef SHOW_RANKS
				d_out<<n.rank2String();
			#endif
				d_out<<"\""<<getOtherAttributes(n)<<"]"<<";\n";
				name=QString("c%1").arg(curr_id);
			}
			setGraphRank(n.getRank()+1,name);
			return name;
		}
		else
			return QString("%1%2").arg((n.isGraphNode()?"g":"c")).arg(curr_id);
	}
public:
	DisplayNodeVisitor(){	}
	virtual void initialize() {
		nodeMap.clear();
		last_id=0;
		ranksList.clear();
		file=new QFile("graph.dot");
		file->remove();
		if (!file->open(QIODevice::ReadWrite))
		{
			out<<"Error openning file\n";
			return;
		}

		dot_out=new QTextStream(file);
		d_out.setCodec("utf-8");
		d_out<<"digraph hadith_graph {\n";
	}
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int /*child_num*/)
	{
		QString s1=getAndInitializeDotNode(n1), s2=getAndInitializeDotNode(n2);
		if (controller->isTop2Bottom())
			d_out<<s1<<"->"<<s2<<";\n";
		else
			d_out<<s2<<"->"<<s1<<";\n";
	}
	virtual bool visit(NarratorNodeIfc & n) //this is enough
	{
		QString s=getAndInitializeDotNode(n);
		displayChainNumsEndingJustAfter(n,s);
	#ifdef DISPLAY_GRAPHNODES_CONTENT
		if (n.isGraphNode())
			out<<n.toString()<<"\n";
	#endif
		return true;
	}
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & n)
	{
		NarratorNodeIfc * current=&n;
		out<<"cycle at ";
		out<<"[";
		QString s=n.CanonicalName();
		out<<s<<",";
		const GraphVisitorController::ParentStack & stack=controller->getParentStack();
		int size=stack.size();
		for (int i=size-1; i>=0; i--)
		{
			current=stack[i];
			if (current==&n && i!=size-1)
				break;
			s=current->CanonicalName();
			out<<s<<",";
		}
		out<<"]\n";
	#if 0
		do
		{
			if (controller->parentStack.isEmpty())
				break;
			controller->parentStack.pop();
			QString s=current->CanonicalName();
			out<<s<<",";
			//qDebug()<<s<<",";
			if (controller->parentStack.isEmpty())
				break;
			current=controller->parentStack.top();
		}while(current!=&n && !current->isNull());
		out<<"]\n";
	#endif
	}
	virtual void finish()
	{
	#ifdef FORCE_RANKS
		QString s;
		int startingRank=(parameters.display_chain_num?0:1);
		int currRank=startingRank,lastRank=startingRank;
		if (ranksList.size()>0)
		{
			while (ranksList[currRank].size()==0)
				currRank++;
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
			d_out<<"{ rank = source;";
			foreach(s,ranksList[currRank])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(lastRank);
			d_out<<"}\n";
			lastRank++;
		}

		for (int rank=currRank+1;rank<ranksList.size()-1;rank++)
		{
			if (ranksList[rank].size()>0)
			{
				d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
				d_out<<QString("r%2 -> r%1 [style=invis];\n").arg(lastRank).arg(lastRank-1);
				d_out<<"{ rank = same;";
				foreach(s,ranksList[rank])
					d_out<<s<<";";
				d_out<<QString("r%1;").arg(lastRank);
				d_out<<"}\n";
				lastRank++;
			}
		}

		int rank=ranksList.size()-1;
		if (rank>startingRank)
		{
			d_out<<QString("r%1 [label=\"%1\"];\n").arg(lastRank);
			d_out<<QString("r%2 -> r%1 [style=invis];\n").arg(lastRank).arg(lastRank-1);
			d_out<<"{ rank = sink;";
			foreach(s,ranksList[rank])
				d_out<<s<<";";
			d_out<<QString("r%1;").arg(lastRank);
			d_out<<"}\n";
		}
	#endif
		d_out<<"}\n";
		delete dot_out;
		file->close();
		delete file;
	}

};

class DisplayNodeVisitorColoredBiography: public DisplayNodeVisitor {
private:
	int index;
protected:
	virtual QString getOtherAttributes(NarratorNodeIfc & n) {
		if (n.hasBiographyIndex(index))
			return ",style=filled, fillcolor=grey";
		else
			return "";
	}
public:
	DisplayNodeVisitorColoredBiography(int biographyIndex) {
		index=biographyIndex;
	}
};
class DisplayNodeVisitorColoredNarrator: public DisplayNodeVisitor {
public:
	typedef QMap<NarratorNodeIfc*,double> DetectedNodesMap;
private:
	DetectedNodesMap & map;
protected:
	virtual QString getOtherAttributes(NarratorNodeIfc & n) {
		DetectedNodesMap::iterator i = map.find(&n);
		if (i!=map.end()) {
			double v=*i;
			QColor r=Qt::lightGray;
			r=r.darker(min(v*1000,100.0));
			unsigned int color=r.toRgb().rgba();
			QByteArray text = QByteArray::number(color,16);
		#ifdef NARRATORHASH_DEBUG
			qDebug()<<"["<<n.CanonicalName()<<"\t"<<v<<"]";
		#endif
			return QString(",style=filled, fillcolor=\"#").append(text.data()).append("\"");
		} else
			return "";
	}
public:
	DisplayNodeVisitorColoredNarrator(DetectedNodesMap & m):map(m) {	}
};
class DisplayLocalNodesVisitor: public DisplayNodeVisitorColoredNarrator {
public:
	DisplayLocalNodesVisitor(DetectedNodesMap & m):DisplayNodeVisitorColoredNarrator(m) {	}
	virtual void initialize() {
		if (controller->isTop2Bottom()) {
			DisplayNodeVisitorColoredNarrator::initialize();
		}
	}
	virtual void finish() {
		if (!controller->isTop2Bottom()) {
			DisplayNodeVisitorColoredNarrator::finish();
		}

	}
};

class HashNodesVisitor: public NodeVisitor
{
private:
	NarratorHash * hash;
public:
	HashNodesVisitor(NarratorHash * hash){
		this->hash=hash;
	}
	void initialize(){ hash->clear();}
	virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc &, int ){	}
	virtual bool visit(NarratorNodeIfc & n) {
		assert(!n.isGroupNode());
		if (n.isGraphNode()) {
			for (int i=0;i<n.size();i++) {
				GroupNode * g=dynamic_cast<GroupNode*>(&n[i]);
				hash->addNode(g);
			}
		} else {
			ChainNarratorNode * c=dynamic_cast<ChainNarratorNode*>(&n);
			GroupNode * g=&c->getGroupNode();
			assert(g!=NULL);
			hash->addNode(g);
		}
		return true;
	}
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & ){}
	virtual void finish(){}
};

class RankCorrectorNodeVisitor:	public NodeVisitor
{
private:
	int highest_rank;
public:
	void initialize()
	{
		highest_rank=0;
	}
	int getHighestRank()
	{
		return highest_rank;
	}
	virtual void visit(NarratorNodeIfc & n1,NarratorNodeIfc & n2, int /*child_num*/)
	{
		int rank1=n1.getRank(), rank2=n2.getRank();
		if (rank1>=rank2)
			rank2=rank1+1;
		n2.setRank(rank2);
		n1.setRank(rank1);
		if (rank2>highest_rank)
			highest_rank=rank2;
	}
	virtual bool visit(NarratorNodeIfc &) { return true; }
	virtual void finishVisit(NarratorNodeIfc & ){ }
	virtual void detectedCycle(NarratorNodeIfc & ){ }
	virtual void finish(){	}
};
class LoopBreakingVisitor: public NodeVisitor
{
private:
	double threshold;
	QList<NarratorNodeIfc *> toDelete;
	void swapThresholds() //swap threshold and parameters.equality_threshold
	{
		double temp=hadithParameters.equality_threshold;
		hadithParameters.equality_threshold=threshold;
		threshold=temp;
	}
	void reMergeNodes(NarratorNodeIfc * n);

public:
	LoopBreakingVisitor(double equality_threshold) {threshold=equality_threshold; }
	virtual void initialize() {  }
	virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int ) {  }
	virtual bool visit(NarratorNodeIfc &) {	return true; }
	virtual void finishVisit(NarratorNodeIfc & ){}
	virtual void detectedCycle(NarratorNodeIfc & n)
	{
		NarratorNodeIfc * current=&n;
		reMergeNodes(current);
		const GraphVisitorController::ParentStack & stack=controller->getParentStack();
		int size=stack.size();
		for (int i=size-1; i>=0; i--)
		{
			current=stack[i];
			if (current==&n && i!=size-1)
				break;
			reMergeNodes(current);
		}
	}
	virtual void finish();
};

class NarratorDetector;


class NarratorGraph{
public:
	class HadithFileDetails {
	public:
		HadithFileDetails(QString fileName) {this->fileName=fileName;}
		HadithFileDetails(QDataStream &in) {deserialize(in);}
		QString fileName;
		//TODO: later add last modified date for check

		void serialize(QDataStream &out) {
			out << fileName;
		}
		void deserialize(QDataStream &in){
			in>>fileName;
		}
	};

	typedef QMap<NarratorNodeIfc *,int> Node2IntMap;
	typedef QMap<int,NarratorNodeIfc *> Int2NodeMap;
	typedef QMap<QString *,int> String2IntMap;
	typedef QMap<int,QString *> Int2StringMap;
	typedef QList<HadithFileDetails> HadithFilesList;
private:
	QList<ChainNarratorNode*> topChainNodes;
	ColorIndices colorGuard;
	NarratorNodesList	top_nodes, all_nodes;

	friend class ColorIndices;
	friend class NarratorDetector;
	friend class HadithDagGraph;
	friend class HadithDagItemModel;

	friend class BiographiesWindow;
	ATMProgressIFC *prg;
	NarratorHash hash;
	bool built; //needed to check if we are to fillNodes

#if 0
	Node2IntMap node2IntMap;
	Int2NodeMap int2NodeMap;
#endif
	unsigned int nodesCount;
	String2IntMap hadith2IntMap;
	Int2StringMap int2HadithMap;
	HadithFilesList hadithFileList;
	friend class NarratorNodeIfc;
	friend class GraphNarratorNode;
	friend class ChainNarratorNode;
	friend class GroupNode;
	friend class NarratorHash;

	int highest_rank;//rank of the deapmost node;
	int getDeapestRank(){return highest_rank;}

	int getSerializationNodeEquivalent(NarratorNodeIfc * n) {
		return n->getId();
	}
	int allocateSerializationNodeEquivalent(NarratorNodeIfc * n) {
		return n->getId();
	}
	NarratorNodeIfc * getDeserializationIntEquivalent(int num) {
		if (num>=0 && num<all_nodes.size())
			return all_nodes[num];
		else
			return NULL;
	}
	void setDeserializationIntEquivalent(int num,NarratorNodeIfc * node) {
		node->setId(num);
		if(getDeserializationIntEquivalent(num)==node)
			return;
		for (int i=nodesCount;i<=num;i++)
			addNode(NULL);
		all_nodes[num]=node;
	}
	int getHadithStringSerializationEquivalent(QString * text) {
		assert(text!=NULL);
		String2IntMap::iterator i=hadith2IntMap.find(text);
		if (i!=hadith2IntMap.end()) {
			return *i;
		} else
			return -1;
	}
	QString * getHadithStringDeserializationIntEquivalent(int num) {
		Int2StringMap::iterator i=int2HadithMap.find(num);
		if (i!=int2HadithMap.end()) {
			return *i;
		} else {
			return NULL;
		}
	}
	void setHadithStringDeserializationIntEquivalent(int num,QString * n) {
		int2HadithMap[num]=n;
	}
	void setHadithStringSerializationEquivalent(QString * n,int num) {
		hadith2IntMap[n]=num;
	}

	friend class LoopBreakingVisitor;
	friend class GraphVisitorController;
	friend class HadithTaggerDialog;
	GraphNarratorNode * mergeNodes(GroupNode & g1,GroupNode & g2,QList<NarratorNodeIfc *> * toDelete=NULL) {
	//if toDelete=NULL, delete what is needed directly, else append to list and postpone deletion
		NarratorNodeIfc * narr1=&g1.getCorrespondingNarratorNode(),
						* narr2=&g2.getCorrespondingNarratorNode();
		bool null1=(narr1==NULL || narr1->isChainNode()),
			 null2=(narr2==NULL || narr2->isChainNode()),
			 sameKey=g1.getKey()==g2.getKey();
		if (sameKey) {
			if (&g1==&g2)
				return narr1->isGraphNode()?(GraphNarratorNode *)narr1:NULL;
			GroupNode & g_node=(null2?g1:g2);
			GroupNode & dlt_g_node=(null2?g2:g1);
			for (int i=0;i<dlt_g_node.size();i++) {
				g_node.addChainNode(this,dlt_g_node[i]);
			}


			if (!null1 && !null2) {
				GraphNarratorNode & graph_node=dynamic_cast<GraphNarratorNode &>(g_node.getCorrespondingNarratorNode());
				GraphNarratorNode * dlt_graph_node= &dynamic_cast<GraphNarratorNode &>(dlt_g_node.getCorrespondingNarratorNode());
				if(&graph_node!=dlt_graph_node) { //not enough also need to merge other corresponding group nodes
					int size=dlt_graph_node->size();
					for (int i=0;i<size;i++) {
						GroupNode * dlt_group=&(*dlt_graph_node)[i];
						if (&dlt_g_node!=dlt_group)
							graph_node.addGroupNode(*dlt_group);
					}
					dlt_graph_node->groupList.clear();
					if (toDelete!=NULL) {
						assert (!toDelete->contains(dlt_graph_node));
						toDelete->append(dlt_graph_node);
					} else {
						all_nodes[dlt_graph_node->getId()]=NULL;
						delete dlt_graph_node;
					}
				}
			}

			if (toDelete!=NULL) {
				assert(!toDelete->contains(&dlt_g_node));
				dlt_g_node.list.clear();
				toDelete->append(&dlt_g_node);
			} else {
				all_nodes[dlt_g_node.getId()]=NULL;
				hash.remove(&dlt_g_node);
				dlt_g_node.list.clear();
				delete &dlt_g_node;
			}
			if (null1 && null2) {
				return new GraphNarratorNode(*this,g_node);
			} else  {
				return dynamic_cast<GraphNarratorNode*>(&g_node.getCorrespondingNarratorNode());
			}
		} else {
			if (null1 && null2) {
				GraphNarratorNode * g=new GraphNarratorNode(*this,g1,g2);
				return g;
			} else if (!null1 && null2) {
				GraphNarratorNode * g=(GraphNarratorNode *)narr1;
				g->addGroupNode(g2);
				return (GraphNarratorNode *)narr1;
			} else if (null1 && !null2) {
				GraphNarratorNode * g=(GraphNarratorNode *)narr2;
				g->addGroupNode(g1);
				return (GraphNarratorNode *)narr2;
			} else if (narr1!=narr2) {
				GraphNarratorNode & g_node=*(GraphNarratorNode *)narr2;
				GraphNarratorNode * dlt_g_node= (GraphNarratorNode *)narr1;
				for (int i=0;i<dlt_g_node->size();i++) {
					g_node.addGroupNode((*dlt_g_node)[i]);
				}
				dlt_g_node->groupList.clear();
				if (toDelete!=NULL) {
					assert (!toDelete->contains(dlt_g_node));
					toDelete->append(dlt_g_node);
				} else {
					all_nodes[dlt_g_node->getId()]=NULL;
					delete dlt_g_node;
				}
				return &g_node;
			} else {
				return (GraphNarratorNode *)narr1;
			}
		}
	}
	GraphNarratorNode * mergeNodes(NarratorNodeIfc & n1,NarratorNodeIfc & n2,QList<NarratorNodeIfc *> * toDelete=NULL) {
	//if toDelete=NULL, delete what is needed directly, else append to list and postpone deletion
		NarratorNodeIfc * narr1=&n1.getCorrespondingNarratorNode(),
						* narr2=&n2.getCorrespondingNarratorNode();
		GroupNode	* group1=NULL,
					* group2=NULL;
		if (narr1->isChainNode()) {
			ChainNarratorNode * c1=dynamic_cast<ChainNarratorNode*>(narr1);
			group1=&c1->getGroupNode();
			if (group1==NULL) { //in principle must not happen
				group1=new GroupNode(*this,(GraphNarratorNode*)c1,c1);
			}
		} else {
			assert(narr1->isGraphNode());
			GraphNarratorNode * graph1=dynamic_cast<GraphNarratorNode*>(narr1);
			assert(graph1->size()>0);
			group1=&(*graph1)[0];
		}
		if (narr2->isChainNode()) {
			ChainNarratorNode * c2=dynamic_cast<ChainNarratorNode*>(narr2);
			group2=&c2->getGroupNode();
			if (group2==NULL) { //in principle must not happen
				group2=new GroupNode(*this,(GraphNarratorNode*)c2,c2);
			}
		} else {
			assert(narr2->isGraphNode());
			GraphNarratorNode * graph2=dynamic_cast<GraphNarratorNode*>(narr2);
			assert(graph2->size()>0);
			group2=&(*graph2)[0];
		}
		assert(group1!=NULL && group2!=NULL);
		return mergeNodes(*group1,*group2,toDelete);
	}
	void unMerge(ChainNarratorNode & c_node) {
		assert(!c_node.isActualNode());
		GroupNode * group=&c_node.getGroupNode();
		NarratorNodeIfc * corresponding=&c_node.getCorrespondingNarratorNode();
		GraphNarratorNode * g=dynamic_cast<GraphNarratorNode*>(corresponding);
		assert(group->size()>0);
		if (group->size()==1) {
			g->groupList.removeOne(group);
			group->setGraphNode((GraphNarratorNode*)&c_node);
		} else {
			assert(group->list.removeOne(&c_node));
			new GroupNode(*this,(GraphNarratorNode*)&c_node,&c_node);
			group->list.removeOne(&c_node);
		}
		if (corresponding->size()==1 && group->size()==1) {
			GroupNode * otherGroup=&(*g)[0];
			if (otherGroup->size()==1) {
				otherGroup->setGraphNode((GraphNarratorNode*)(&(*otherGroup)[0]));
				all_nodes[g->getId()]=NULL;
				delete g;
			}
		}
		if (top_nodes.contains(corresponding)) {
			if (c_node.isFirst())
				top_nodes.append(&c_node);
		}
	}
	void transform2ChainNodes(ChainsContainer &chains) {
	//pre-condition: chain contains the valid chains extracted from the hadith
		prg->setCurrentAction("Creating Nodes");
		int num_chains=chains.count();
		for (int chain_num=0;chain_num<num_chains;chain_num++)
		{
			int size=chains.at(chain_num)->m_chain.count();
			ChainNarratorNode* last=NULL;
			for (int j=0;j<size;j++)
			{
				if (chains.at(chain_num)->m_chain[j]->isNarrator())
				{
					Narrator *n=(Narrator *)(chains.at(chain_num)->m_chain[j]);
					ChainNarratorNode* current= new ChainNarratorNode(*this,n,0,chain_num);
					if (last !=NULL)
					{
						last->previous=current;
						current->next=last;
					}
					last=current;
				}
			}
			if (last!=NULL)
				top_nodes.append(last);
			//code below to correct index
			int index=0;
			for (ChainNarratorNode * current=last;current !=NULL && !current->isNull();current=&current->nextInChain())
			{
				current->setIndex(index);
				index++;
			}
			prg->report(100.0*chain_num/num_chains+0.5);
		}
	//postcondition: Narrator's are transformed to ChainNarratorNode's and linked into
	//				 chains and top_nodes stores the link to the first node of each chain
	}
#ifdef HASH_GRAPH_BUILDING
	static bool mustMerge(GroupNode * g_node,ChainNarratorNode * c_node) {
		//TODO: we must add also another check that makes sure no nodes in chains of this group have been merged after this possible merge (but very expensive)

		int chainNum=c_node->getChainNum();
		int c_index=c_node->getIndex();
		int least,highest;
		ChainNarratorNode & c2_node=g_node->getChainNodeInChain(chainNum);
		if (!c2_node.isNull())
			return false;
		NarratorNodeIfc * n=&g_node->getCorrespondingNarratorNode();
		if (n!=NULL && n->isGraphNode()) {
			GraphNarratorNode * g=(GraphNarratorNode *)n;
			ChainNarratorNode & c2_node=g->getChainNodeInChain(chainNum);
			if (!c2_node.isNull())
				return false;

			least=min(c_index,g->getLowestIndex());
			highest=max(c_index,g->getHighestIndex());
		} else {
			least=min(c_index,g_node->getLowestIndex());
			highest=max(c_index,g_node->getHighestIndex());
		}
		//check if it is within radius
		if (highest-least<=hadithParameters.equality_radius)
			return true;
		return false;
	}
	class BuildAction: public NarratorHash::FoundAction {
	private:
		NarratorGraph * graph;
		GraphNodeItem * node;
		QList<NarratorNodeIfc *> * toDelete;
		bool merged;
	private:
		ChainNarratorNode * cn() {return dynamic_cast<ChainNarratorNode*>(node);}
		GroupNode * gn() {return dynamic_cast<GroupNode*>(node);}
		bool isTotalEquality(double val) {
		#if 0
			return (val==1.0);
		#else
			double a=absVal(1.0-val);
			assert(a>=0);
			return a<0.0001;
		#endif
		}
	public:
		BuildAction(GraphNodeItem * aNode,NarratorGraph * graph,QList<NarratorNodeIfc *> * toDelete=NULL){
			node=aNode;
			this->graph=graph;
			this->toDelete=toDelete;
			merged=false;
		}

		virtual void action(const QString &, GroupNode * hashedNode, double val) {
			if (
			#if 0
					isTotalEquality(val)
			#else
					node->getKey()==hashedNode->getKey()
			#endif
				&& !merged) {
				assert(isTotalEquality(val));
				if (node->isChainNode())
					merged=mustMerge(hashedNode,cn());
				else
					merged=true;
				if (merged) {
					GroupNode * group=(node->isChainNode()?&cn()->getGroupNode():gn());
					if (group==NULL) {
						hashedNode->addChainNode(graph,*cn());
						if (hashedNode->getCorrespondingNarratorNode().isChainNode()) {
							new GraphNarratorNode(*graph,*hashedNode);
						}
					} else {
						graph->mergeNodes(*hashedNode,*group,toDelete); //since both are not null, the one to be deleted is the second which is already not in the hash not the first
					}
				}
			}
		}
		bool isMerged() {return merged;}
	};
	class MergeAction: public NarratorHash::FoundAction {
	private:
		NarratorGraph * graph;
		GraphNodeItem * node;
		QList<NarratorNodeIfc *> * toDelete;
		bool merged;
	private:
		ChainNarratorNode * cn() {return dynamic_cast<ChainNarratorNode*>(node);}
		GroupNode * gn() {return dynamic_cast<GroupNode*>(node);}
	public:
		MergeAction(GraphNodeItem * aNode,NarratorGraph * graph,QList<NarratorNodeIfc *> * toDelete=NULL){
			node=aNode;
			this->graph=graph;
			this->toDelete=toDelete;
			merged=false;
		}
		virtual void action(const QString &, GroupNode * hashedNode, double val) {
			if (!merged && val>hadithParameters.equality_threshold) {
				if (node->isChainNode())
					merged=mustMerge(hashedNode,cn());
				else
					merged=true;
				if (merged) {
					GroupNode * group=(node->isChainNode()?&cn()->getGroupNode():gn());
					if (group==NULL) {
						group=new GroupNode(*graph,NULL,cn());
					#if 0
						NarratorNodeIfc & n=hashedNode->getCorrespondingNarratorNode();
						if (n.isGraphNode()) {
							GraphNarratorNode & g=(GraphNarratorNode &)n;
							for (int i=0;i<g.size();i++) {
								assert(g[i].getKey()!=node->getKey());
							}
						}
					#endif
					}
					assert(group->getKey()!=hashedNode->getKey());
					graph->mergeNodes(*hashedNode,*group,toDelete); //since both are not null, the one to be deleted is the second which is already not in the hash not the first
				}
			}
		}
		bool isMerged() {return merged;}
	};
	void mergeGraphs(NarratorHash & otherHash) {
		QList<NarratorNodeIfc *> deleteList;
		prg->setCurrentAction("Merging Nodes");
		prg->report(0);
		unsigned int bit=colorGuard.getNextUnused();
		colorGuard.use(bit);
		int i=0,size=otherHash.size();
		for (NarratorHash::HashTable::iterator itr=otherHash.begin();itr!=otherHash.end();itr++ ) {
			GroupNode * node=(*itr).node;
			if (!node->isVisited(bit)) {
				node->setVisited(bit);
				int d1=deleteList.size();
				BuildAction bAction(node,this,&deleteList);
				performActionToExactCorrespondingNodes(node,bAction); //search for matches for this key only
				if (!bAction.isMerged()) {
					//1-merge with node that it is equal to
					MergeAction mAction(node,this,&deleteList);
					performActionToAllCorrespondingNodes(node,mAction);
					//2-add to hash
					hash.addNode(node);
				}
				int d2=deleteList.size();
				if (d2>d1) {
					assert(d2-d1<=2);
					NarratorNodeIfc * n=deleteList.last(); //even if 2 nodes have been deleted the last is the group node
					if (n->isGroupNode())
						hash.remove(dynamic_cast<GroupNode*>(n));
				}
			}
			prg->report(100.0*i/size+0.5);
			i++;
		}
		for (int i=0;i<deleteList.size();i++) {
			NarratorNodeIfc * n=deleteList[i];
			all_nodes[n->getId()]=NULL;
			delete n;
		}

		colorGuard.unUse(bit);
		prg->report(100);
	}
#else
	GraphNarratorNode & mergeNodes(ChainNarratorNode & n1,ChainNarratorNode & n2) {
		//assert(!((NarratorNodeIfc &)n1).isGraphNode());
		//assert(!((NarratorNodeIfc &)n2).isGraphNode());
		NarratorNodeIfc & narr1=n1.getCorrespondingNarratorNode(),
						& narr2=n2.getCorrespondingNarratorNode();
		bool graph1=narr1.isGraphNode(),
			 graph2=narr2.isGraphNode();
		if (!graph1 && !graph2)
		{
			GraphNarratorNode * g=new GraphNarratorNode(*this,n1,n2);
		#if 0
			out<<g->toString()<<"\n";
		#endif
			return *g;
		}
		else if (graph1 && !graph2)
		{
			//assert(&n2==&narr2);
			((GraphNarratorNode &)narr1).addChainNode(this,n2);
		#if 0
			out<<narr1.toString()<<"\n";
		#endif
			return (GraphNarratorNode &)narr1;
		}
		else if (!graph1 && graph2)
		{
			//assert(&n1==&narr1);
			((GraphNarratorNode &)narr2).addChainNode(this,n1);
		#if 0
			out<<narr2.toString()<<"\n";
		#endif
			return (GraphNarratorNode &)narr2;
		}
		else if (&narr1!=&narr2)
		{
			//assert(narr1.isGraphNode() && narr2.isGraphNode());
			GraphNarratorNode & g_node=(GraphNarratorNode &)narr2;
			GraphNarratorNode * dlt_g_node= &(GraphNarratorNode &)narr1;
			ChainNodeIterator itr=narr1.begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode & c_node=*itr;
				g_node.addChainNode(this,c_node);
			}
		#if 0
			out<<g_node.toString()<<"\n";
		#endif
			//assert(&n1.getCorrespondingNarratorNode()!=&narr1);
			removeNode(dlt_g_node);
			delete dlt_g_node;
			return g_node;
		}
		else
			return (GraphNarratorNode &)narr1;
	}
#endif
	void buildGraph(int split=-1) {
	#ifdef HASH_GRAPH_BUILDING
		int num_chains=top_nodes.size();
		prg->setCurrentAction("Merging Nodes");
		prg->report(0);
		int minChains=0;
		if (split>0)
			minChains=split;
		int total=num_chains-minChains;
		for (int i=minChains;i<num_chains;i++) {
			ChainNarratorNode & c1= *(ChainNarratorNode *)top_nodes[i]; //since at this stage all are ChainNode's
			ChainNarratorNode * n1=&c1; //start from beginning of c1
			for (;!n1->isNull();n1=&(n1->nextInChain())) {
				//check if there is a group node equal to it fully and merge them together.
			#ifdef DEBUG_BUILDGRAPH
				qDebug()<<n1->toString();
			#endif
				BuildAction bAction(n1,this);
				performActionToExactCorrespondingNodes(n1,bAction); //search for matches for this key only
				if (!bAction.isMerged()) {
					//1-merge with node that it is equal to
					MergeAction mAction(n1,this);
					performActionToAllCorrespondingNodes(n1,mAction);
					//2-add to hash
					GroupNode * g_node=NULL;
					if (mAction.isMerged()) {
						g_node=&n1->getGroupNode();
						assert(g_node!=NULL); //was merged already
					} else {
						g_node=new GroupNode(*this,(GraphNarratorNode*)n1,n1);
					}
					//hash.addNode(g_node);
				}
			}
			prg->report(100.0*(total-(num_chains-i))/total+0.5);
		}
		prg->report(100);
	#else
		//note: compares chain by chain then moves to another
		//TODO: check how to remove duplication when it occurs in the contents of a graph node, if not taken care when inserting
		int radius=hadithParameters.equality_radius;
		double threshold=hadithParameters.equality_threshold;
		int num_chains=top_nodes.size();
		prg->setCurrentAction("Merging Nodes");
		prg->report(0);
		int maxChains=num_chains;
		if (split>0)
			maxChains=split;
		for (int i=0;i<maxChains;i++)
		{
			//assert(!top_nodes[i]->isGraphNode());
			ChainNarratorNode & c1= *(ChainNarratorNode *)top_nodes[i]; //since at this stage all are ChainNode's
			int minChains=i+1;
			if (split>0)
				minChains=split+1; //TODO: check if must start from 'split'
			for (int k=minChains;k<num_chains;k++)
			{
				ChainNarratorNode & c2= *(ChainNarratorNode *)top_nodes[k];
				int needle=0;
				ChainNarratorNode * n1=&c1; //start from beginning of c1
				int u=0,offset=-1;
				for (;!n1->isNull();n1=&(n1->nextInChain()))
				{
					NarratorNodeIfc & g_node=n1->getCorrespondingNarratorNode();//get the graph node corresponding to n1, or n1 if no such exists
					ChainNarratorNode & lastMergedNode //the node we should start comparing from after it
							=g_node.getChainNodeInChain(k);//return the chain k node in the graph node if it exists

					if (!lastMergedNode.isNull())
						offset=max(offset, //previous offset which may be also increased since 2 were found equal
								   lastMergedNode.getIndex());//index of lastMergedNode, which we must skip
					u=max(offset+1,needle-radius); //the iterator over nodes of chain[k] for comparison
					u=u>0?u:0;
					bool match=false;

					int increment=u-offset; //increment that we should add to start at u
					ChainNarratorNode * n2=(lastMergedNode.isNull()
											? &(c2+u) //start iteration at u (from beginning +u)
											:&(lastMergedNode+increment)); //start iteration at u ( lastNode+(u-index(lastNode)) )
					for(;u<needle+radius && !n2->isNull();u++,n2=&(n2->nextInChain())) //we are within bound of radius and of the chain size (if null => finished)
					{
						Narrator & n1_ref=n1->getNarrator();
						Narrator & n2_ref=n2->getNarrator();
						double eq_val=equal(n1_ref,n2_ref);
					#ifdef DEBUG_BUILDGRAPH
						qDebug()<<n1_ref.getString()<<"["<<i<<","<<n1->getIndex()<<"]Versus["<<n2_ref.getString()<<"["<<k<<","<<n2->getIndex()<<"]\t"<<eq_val<<"\n";
					#endif
						if (eq_val>=threshold)
						{
						#ifdef DEBUG_BUILDGRAPH
							qDebug()<<"\t=>merge\n";
						#endif
							mergeNodes(*n1,*n2);
							offset=u;	//this is matched, we must skip it in search for match for next node in c1
							needle=u+1; //since the node is matched, we move to match the next
							match=true;
							break;
						}
					}
					if (!match)
						needle++;
				}
			}
			prg->report(100.0*i/maxChains);
		}
		prg->report(100);
	#endif
	}
	void computeRanks() {
	#if 0
		for (int trials=0;trials<2;trials++)
		{
			for (int i=0;i<top_nodes.size();i++)
			{
				for (int j=0;j<top_nodes[i]->size();j++)
				{
					ChainNarratorNode * last=&(*top_nodes[i])[j];
					for (ChainNarratorNode * current=last;!current->isNull();current=&current->nextInChain())
					{
						int rank1=last->getCorrespondingNarratorNode().getAutomaticRank(),
							rank2=current->getCorrespondingNarratorNode().getAutomaticRank();
						if (rank1>=rank2)
							rank2=rank1+1;
						current->setRank(rank2);
						//current->getCorrespondingNarratorNode().setRank(rank2);
						//last->getCorrespondingNarratorNode().setRank(rank1);
						last->setRank(rank1);
						if (rank2>highest_rank)
							highest_rank=rank2;
						last=current;
					}
				}
			}
		}
	#else
		RankCorrectorNodeVisitor r;
		GraphVisitorController c(&r,this);
		prg->setCurrentAction("Computing Ranks");
		prg->report(0);
		BFS_traverse(c);
		prg->report(50);
		BFS_traverse(c);
		prg->report(100);
		highest_rank=r.getHighestRank();
	#endif
	}
	void breakManageableCycles() {
		const double step=hadithParameters.equality_delta;
		const int num_steps=3;
		prg->setCurrentAction("Breaking Cycles");
		prg->report(0);
		for (int i=1;i<=num_steps;i++)
		{
		#ifdef DISPLAY_NODES_BEING_BROKEN
			qDebug()<<"--";
		#endif
			double threshold=hadithParameters.equality_threshold+i*step;
			//qDebug()<<"---Break----";
			LoopBreakingVisitor l(threshold);
			GraphVisitorController c(&l,this);
			DFS_traverse(c);
			prg->report(i/num_steps*100+0.5);
		}

	}
	void correctTopNodesList() {
		prg->setCurrentAction("Correct TopList");
		prg->report(0);
		NarratorNodesList new_top_list;
		int size=top_nodes.size();
		for (int i=0;i<size;i++)
		{
			//ChainNarratorNode & c=(ChainNarratorNode &)*top_nodes[i];
			//if(c.isFirst()) {
				NarratorNodeIfc * g=&top_nodes[i]->getCorrespondingNarratorNode();
				//assert(!g->isNull());//it can only be null if c was not a ChainNarratorNode, anyways, does not hurt to check
				if (g->isGraphNode()) {
					if (!new_top_list.contains(g)) //might have been already added
						new_top_list.append(g);
				} else
					new_top_list.append(g);
			//}
			prg->report(100.0*i/size+0.5);
		}
		prg->report(100.0);
		top_nodes=new_top_list; //TODO: try reduce copy cost
	}
	void hashNodes() {
		//qDebug()<<"---Hash----";
		prg->setCurrentAction("Hash Nodes");
		prg->report(0);
		HashNodesVisitor visitor(&hash);
		GraphVisitorController c(&visitor,this);
		DFS_traverse(c);
		prg->report(100);
	}
	void printChains(){
		QList<int> chainNums;
		QList<ChainNarratorNode *> topOfChain;
		for (int i=0;i<all_nodes.size();i++) {
			NodeIterator itr=all_nodes[i]->begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode * c=dynamic_cast<ChainNarratorNode*>(&*itr);
				int chainNum=c->getChainNum();
				if (!chainNums.contains(chainNum)) {
					chainNums.append(chainNum);
					while (!c->isFirst()) {
						c=&c->prevInChain();
					}
					topOfChain.append(c);
				}
			}
		}
		for (int i=0;i<topOfChain.size();i++) {
			qDebug()<<"-----"<<chainNums[i]<<"------\n";
			ChainNarratorNode* c=topOfChain[i];
			while (!c->isNull()) {
				qDebug()<<getSerializationNodeEquivalent(c)<<"\t"<< c->CanonicalName();
				c=&c->nextInChain();
			}
			qDebug()<<"\n";
		}
	}
	void printInAll(int num){
		qDebug()<<"";
		for (int i=0;i<all_nodes.size();i++) {
			NodeIterator itr=all_nodes[i]->begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode* c=dynamic_cast<ChainNarratorNode*>(&*itr);
				int chainNum=c->getChainNum();
				if (chainNum==num) {
					qDebug()<<getSerializationNodeEquivalent(c)<<"\t"<< c->CanonicalName();
				}
			}
		}
		qDebug()<<"---";
	}
	void removeDuplicatesFromAllNodes() {
		for (int i=0;i<all_nodes.size();i++) {
			for (int j=i+1;j<all_nodes.size();j++) {
				if (all_nodes[i]==all_nodes[j]){
					qDebug()<<all_nodes[j]->CanonicalName();
					all_nodes.removeAt(j);
					j--;
				}
			}
		}
	}
	void removeDuplicatesFromTopNodes() {
		int topSize=top_nodes.size();
		for (int i=0;i<topSize;i++) {
			if (top_nodes[i]->isGraphNode()) {
				bool hasFirst=false;
				NodeIterator itr=top_nodes[i]->begin();
				for (;!itr.isFinished();++itr) {
					ChainNarratorNode* c=dynamic_cast<ChainNarratorNode*>(&*itr);
					if (c->isFirst()) {
						hasFirst=true;
						break;
					}
				}
				if (!hasFirst) {
					top_nodes.removeAt(i);
					i--;
					topSize--;
				}
			} else {
				ChainNarratorNode * c=(ChainNarratorNode *)top_nodes[i];
				if (!c->isFirst()) {
					top_nodes.removeAt(i);
					i--;
					topSize--;
				}
			}
		}
	}

	NodeIterator findEquivalent(ChainNarratorNode * c,NarratorNodeIfc * n) {
		NodeIterator itr=n->begin();
		for (;!itr.isFinished();++itr) {
			ChainNarratorNode* c2=dynamic_cast<ChainNarratorNode*>(&*itr);
			if (c->getNarrator().getStart()==c2->getNarrator().getStart() && c->getNarrator().getEnd()==c2->getNarrator().getEnd()) {
				return itr;
			}
		}
		out<<"conflict at:"<<c->CanonicalName()<<"\t"<<n->CanonicalName()<<"\n";
		return NodeIterator::null;
	}
	bool areEqual(NarratorNodeIfc * n1,NarratorNodeIfc * n2) {
		for (int f=0;f<2;f++) {
			NodeIterator itr=n1->begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode* c1=dynamic_cast<ChainNarratorNode*>(&*itr);
				NodeIterator j2=findEquivalent(c1,n2);
				if (j2==NodeIterator::null)
					return false;
			}
			swap(n1,n2);
		}
		return true;
	}
	NarratorNodeIfc * findEquivalent(NarratorNodeIfc * n,QList<NarratorNodeIfc *> & list) {
		for (int i=0;i<list.size();i++) {
			NarratorNodeIfc * n2=list[i];
			if (areEqual(n,n2))
				return n2;
		}
		return NULL;
	}
	bool equalHelper(NarratorNodeIfc * n1,NarratorNodeIfc * n2) {
		if (n1->isNull() || n2->isNull())
			return (n1->isNull() && n2->isNull());
		NodeIterator itr=n1->begin();
		for (;!itr.isFinished();++itr) {
			ChainNarratorNode* c1=dynamic_cast<ChainNarratorNode*>(&*itr);
			NodeIterator j=findEquivalent(c1,n2);
			if (j.isNull())
				return false;
			NarratorNodeIfc * child1=&itr.getChild();
			NarratorNodeIfc * child2=&j.getChild();
			if (!equalHelper(child1,child2))
				return false;

		}
		return true;
	}

	void init(int split=-1) {
	#ifdef HASH_GRAPH_BUILDING
		if (split<0)
	#endif
			buildGraph(split);
		correctTopNodesList();
		if (hadithParameters.break_cycles){
			breakManageableCycles();
			removeDuplicatesFromTopNodes();
		}
		//hashNodes();
		//removeDuplicatesFromAllNodes();
		computeRanks();
		built=true;
	}
	void clearCachedChildrenParents() {
		int size=all_nodes.size();
		for (int i=0;i<size;i++) {
			NarratorNodeIfc * n=all_nodes[i];
			if (n!=NULL && n->isGraphNode()) {
				GraphNarratorNode * g=dynamic_cast<GraphNarratorNode *>(n);
				g->clearCache();
			}
		}
	}
	void clearStructures() {
		top_nodes.clear();
		all_nodes.clear();
		int2HadithMap.clear();
		hadith2IntMap.clear();
		hadithFileList.clear();
		hash.clear();
		nodesCount=1;
		highest_rank=-1;
	}
	int splitTopList() {
		int largestChainNum=-1;
		int allSize=top_nodes.size();
		for (int i=0;i<allSize;i++) {
			if (top_nodes[i]->isGraphNode()) {
				NodeIterator itr=top_nodes[i]->begin();
				for (;!itr.isFinished();++itr) {
					ChainNarratorNode* c=dynamic_cast<ChainNarratorNode*>(&*itr);
					if (c->isFirst()) {
						top_nodes.append(c);
						int chain=c->getChainNum();
						if (chain>largestChainNum)
							largestChainNum=chain;
					}
				}
				top_nodes.removeAt(i);
				i--;
				allSize--;//although the size has increased in practice no need to check those that we added here
			} else {
				int chain=((ChainNarratorNode*)top_nodes[i])->getChainNum();
				if (chain>largestChainNum)
					largestChainNum=chain;
			}
		}
	#if 1 //TODO: not equal always => top_nodes not totally correct
		//qDebug()<<"top= "<<top_nodes.size()<<"\tlargest+1="<<largestChainNum+1;
		if(top_nodes.size()!=largestChainNum+1){
			removeDuplicatesFromTopNodes();
			qDebug()<<"top= "<<top_nodes.size()<<"\tlargest+1="<<largestChainNum+1;
			assert(top_nodes.size()==largestChainNum+1);
		}
		return top_nodes.size();
	#endif
		//post-condition: top_nodes contains the chain nodes instead of the graph nodes (i.e. opposite of correctTopNodes() )
		// return value=new size of top_nodes
	}
	void shiftChainsBy(int pos) {
		int allSize=top_nodes.size();
		for (int i=0;i<allSize;i++) {
			NodeIterator itr=top_nodes[i]->begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode* c=dynamic_cast<ChainNarratorNode*>(&*itr);
				if (c->isFirst()) {
					for (;!c->isNull();c=&(c->nextInChain()))
						c->setChainNum(c->getChainNum()+pos);
				}
			}
		}
	}
	void addNodesToCurrentGraph(const NarratorNodesList & nodes) {
		for (int i=0;i<nodes.size();i++) {
			if (nodes[i]!=NULL) {
				nodes[i]->setId(nodesCount);
				addNode(nodes[i]);
			}
		}
	}
	void printAllNodesList() {
		for (int i=0;i<all_nodes.size();i++) {
			NarratorNodeIfc *n=all_nodes[i];
			if (n==NULL)
				qDebug()<<i<<": NULL";
			else
				qDebug()<<i<<": "<<n->toString();
		}
	}
	void checkAllGroupHaveCorresponding(){
		for (int i=0;i<all_nodes.size();i++){
			if (all_nodes[i]!=NULL && all_nodes[i]->isGroupNode()){
				GroupNode * n=(GroupNode *)all_nodes[i];
				if(&n->getCorrespondingNarratorNode()==NULL)
					qDebug()<<i<<n->toString();
			}
		}
	}
	bool checkForNode(GroupNode * g){
		for (int i=0;i<all_nodes.size();i++){
			if (all_nodes[i]!=NULL && all_nodes[i]->isGraphNode()){
				GraphNarratorNode * n=(GraphNarratorNode *)all_nodes[i];
				for (int j=0;j<n->size();j++){
					if (&(*n)[j]==g) {
						qDebug()<<n->getId();
						assert(all_nodes[n->getId()]==n);
						assert(all_nodes[g->getId()]==g);
						return true;
					}
				}
			}
		}
		return false;
	}

	class DFS_Helper {
	private:
		GraphVisitorController & visitor;
		NarratorNodeIfc * node;
		int direction;
		int maxLevels;
	private:
		inline void continueTraversal(NarratorNodeIfc & n,int levelsLeft=-1) {
		#if 0
			if(!all_nodes.contains(&n)) {
				all_nodes.append(&n);
				qDebug()<<"["<<n.CanonicalName()<<"]";
			}
		#elif 0
			assert(all_nodes.contains(&n));
		#endif
		#ifdef DEBUG_DFS_TRAVERSAL
			int size=n.size();
			qDebug()<<"parent:"<<n.toString()<<" "<<size<<"\n";
		#endif
			//n.size() instead of saved variable bc in case LoopBreakingVisitor does change thru the traversal
			NodeIterator itr=(direction>0?n.childrenBegin():n.parentsBegin());
			for (;!itr.isFinished();++itr) {
				NarratorNodeIfc & c=(direction>0?itr.getChild():itr.getParent());
				int i=0,j=0;
				if (itr.isChainIterator()) {
					i=itr.getGroupIndex();
					j=itr.getChainIndex();
				} else {
					i=itr.getIndex();
				}
			#ifdef DEBUG_DFS_TRAVERSAL
				qDebug()<<n.CanonicalName()<<".child("<<i<<","<<j<<"):"<<(!c.isNull()?c.CanonicalName():"null")<<"\n";
			#endif
				if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i,j))
				{
					bool prev_visited=visitor.isPreviouslyVisited(c);
					visitor.visit(n,c,i,j);
					if (!prev_visited) {
						if (levelsLeft<0) //i think this reduces the stack size, but i am not sure
							traverse(c);
						else
							traverse(c,levelsLeft);
					}
				}
			}
		}
		void traverse(NarratorNodeIfc & n,int levelsLeft) {
			if (visitor.visit(n)) {
				if (levelsLeft>0) {
					levelsLeft--;
				} else if (levelsLeft==0) {
					visitor.finishVisit(n);
					return;
				}
				continueTraversal(n,levelsLeft);
			}
			visitor.finishVisit(n);
		}
		void traverse(NarratorNodeIfc & n) {
			if (visitor.visit(n))
				continueTraversal(n);
			visitor.finishVisit(n);
		}

	public:
		DFS_Helper(GraphVisitorController & aVisitor):visitor(aVisitor),direction(1),maxLevels(-1) {
		//traversal of all graph
			NarratorGraph * graph=visitor.getGraph();
			assert(graph!=NULL);
			node=NULL;
		}
		DFS_Helper(GraphVisitorController & aVisitor, NarratorNodeIfc * node,int direction=1,int levels=-1):visitor(aVisitor),maxLevels(levels){
		//if direction>0, we go to child,else to parent
		//maxLevels = level after which we stop traversing further, if maxLevels<0, => infinite
			this->direction=direction;
			this->node=node;
			assert(node!=NULL);
		}
		void operator()() {
			visitor.initialize(direction);
			NarratorGraph * graph=visitor.getGraph();
			if (node==NULL) {
				assert(graph!=NULL);
				for (int i=0; i<graph->top_nodes.size();i++) {
					NarratorNodeIfc * node=graph->top_nodes[i];
					if (!visitor.isPreviouslyVisited(*node))
						traverse(*node);
				}
			} else {
				traverse(*node,maxLevels);
			}
			visitor.finish();
		}
	};

protected:
	void addNode(NarratorNodeIfc * node) { //TODO: check all_nodes is used correctly, not modified in another place
		all_nodes.push_back(node);
		assert(node==NULL || node->getId()==nodesCount);
		nodesCount++;
	}
	void removeNode(NarratorNodeIfc * node) {
		int id=node->getId();
		assert(all_nodes[id]==node);
		all_nodes[id]=NULL;
		if (node->isGroupNode())
			hash.remove(dynamic_cast<GroupNode*>(node));
	}

public:
	NarratorGraph(ChainsContainer & chains, ATMProgressIFC *prg):hash(this) {
		built=false;
		nodesCount=1;
		this->prg=prg;
		colorGuard.setGraph(this);
		all_nodes.push_back(NULL);
		transform2ChainNodes(chains);
		init();
		//printAllNodesList();
	}

	void mergeWith(NarratorGraph * graph2) {
		built=false;
		graph2->built=false;
		clearCachedChildrenParents();
		graph2->clearCachedChildrenParents();
		int numChain1=splitTopList();
		int numChain2=graph2->splitTopList();
		graph2->shiftChainsBy(numChain1);
		top_nodes.append(graph2->top_nodes);
		addNodesToCurrentGraph(graph2->all_nodes);
		assert(top_nodes.size()==numChain1+numChain2);
		String2IntMap::iterator i=graph2->hadith2IntMap.begin();
		for (;i!=graph2->hadith2IntMap.end();i++) {
			QString * key=i.key();
			int index=i.value();
			QString fileName=graph2->hadithFileList[index].fileName;
			setFileName(key,fileName);
		}
	#ifdef HASH_GRAPH_BUILDING
		mergeGraphs(graph2->hash);
	#endif
		init(numChain1);
		graph2->clearStructures();
	}
	void setFileName(QString * text, QString fileName) {
		assert(text!=NULL);
		int size=hadithFileList.size();
		setHadithStringDeserializationIntEquivalent(size,text);
		setHadithStringSerializationEquivalent(text, size);
		hadithFileList.append(HadithFileDetails(fileName));
	}
	void DFS_traverse(GraphVisitorController & visitor) {
		DFS_Helper d(visitor);
		d();
	}
	void BFS_traverse(GraphVisitorController & visitor){
		BFS_traverse(visitor,1);
	}
	static void DFS_traverse(NarratorNodeIfc & n,GraphVisitorController & visitor,int maxLevels=-1,int direction=1) {
		//if direction>0, we go to child,else to parent
		//maxLevels = level after which we stop traversing further, if maxLevels<0, => infinite
		DFS_Helper d(visitor,&n,direction,maxLevels);
		d();
	}
	static void BFS_traverse(GraphVisitorController & visitor, int maxLevels,NarratorNodeIfc* node=NULL,int direction=1)
	{//if direction>0, we go to child,else to parent
	 //if node==NULL, we start by top_nodes (direction is disregarded and assumed as child), else by the specified node (direction is used)
	 //maxLevels = level after which we stop traversing further, if maxLevels<0, => infinite

		assert(visitor.getGraph()!=NULL || node!=NULL); //use controller graph or nodes we have
		visitor.initialize(direction);
		QQueue<NarratorNodeIfc *> queue;
		int size;
		if (node==NULL) {
			NarratorGraph * graph=visitor.getGraph();
			size=graph->top_nodes.size();
			for (int i=0; i<size;i++) {
				NarratorNodeIfc * tNode=graph->top_nodes[i];
				queue.enqueue(tNode);
			}
			direction=1; //if we are starting at top nodes, it is understood that direction is toward children
		} else {
			size=1;
			queue.enqueue(node);
		}
		int levelCount=0;
		int numNodesPerLevel=size;
		while (!queue.isEmpty()) {
			if (maxLevels>=0){ //if we are interested to stop after some level
				if (numNodesPerLevel==0) {
					levelCount++;
					if (levelCount>maxLevels) {
						visitor.finish();
						return;
					}
					numNodesPerLevel=queue.size();
				}
				numNodesPerLevel--; //we check then decrement, to make sure we are visiting the last valid node
			}
			NarratorNodeIfc & n=*(queue.dequeue());
			if (visitor.visit(n)) {
				if (maxLevels>=0 && levelCount==maxLevels) { //if we are on last level no need to enque its children or parents
					visitor.finishVisit(n);
					continue;
				}
			#ifdef DEBUG_BFS_TRAVERSAL
				int size=n.size();
				qDebug()<<(direction>0?"parent:":"child:")<<n.toString()<<" "<<size<<"\n";
			#endif
				NodeIterator itr=(direction>0?n.childrenBegin():n.parentsBegin());
				for (;!itr.isFinished();++itr) {
					NarratorNodeIfc & c=(direction>0?itr.getChild():itr.getParent());
					int i=0,j=0;
					if (itr.isChainIterator()) {
						i=itr.getGroupIndex();
						j=itr.getChainIndex();
					} else {
						i=itr.getIndex();
					}
				#ifdef DEBUG_BFS_TRAVERSAL
					qDebug()<<n.CanonicalName()<<(direction>0?".child(":".parent(")<<i<<","<<j<<"):"<<(!c.isNull()?c.CanonicalName():"null")<<"\n";
				#endif
					if (!c.isNull() && !visitor.isPreviouslyVisited(n, c,i,j)) {
						#ifdef DEBUG_BFS_TRAVERSAL
							qDebug()<<"\t-->traversed\n";
						#endif
						bool prev_visited=visitor.isPreviouslyVisited( c);
						visitor.visit(n,c,i,j);
						if (!prev_visited)
							queue.enqueue(&c);
					}
				}
			}
			visitor.finishVisit(n);
		}
		visitor.finish();
	}
	GraphNodeItem * getNodeMatching(Narrator & n) {
	#if 0
		double highest_equality=0;
		ChainNarratorNode * correspondingNode=NULL;
		for (int i=0;i<all_nodes.size();i++) {
			for (int j=0;j<all_nodes[i]->size();j++) {
				ChainNarratorNode * c=&(*all_nodes.at(i))[j];
				Narrator * n2= &c->getNarrator();
				double curr_equality=equal(n,*n2);
				if (curr_equality>highest_equality){
					highest_equality=curr_equality;
					correspondingNode=c;//all_nodes[i];
				}
			}
		}
		if (highest_equality>=hadithParameters.equality_threshold)
			return correspondingNode;
		else
			return NULL;
	#else
		return hash.findCorrespondingNode(&n);
	#endif
	}
	void performActionToAllCorrespondingNodes(Narrator * n, NarratorHash::FoundAction & visitor) {
		hash.performActionToAllCorrespondingNodes(n,visitor);
	}
	void performActionToAllCorrespondingNodes(GraphNodeItem * n, NarratorHash::FoundAction & visitor) {
		hash.performActionToAllCorrespondingNodes(n,visitor);
	}
	void performActionToExactCorrespondingNodes(GraphNodeItem * n, NarratorHash::FoundAction & visitor) {
		hash.performActionToExactCorrespondingNodes(n,visitor);
	}

	void serialize(QDataStream & streamOut) {
	#define SERIALIZE_STOP -2
		int size=hadithFileList.size();
		streamOut<<size;
		for (int i=0;i<size;i++) {
			hadithFileList[i].serialize(streamOut);
		}
	#ifdef PROGRESS_SERIALZATION
		prg->setCurrentAction("Serializing");
		prg->report(0);
		int allSize=all_nodes.size();
		int total=allSize+top_nodes.size();
		streamOut<<total;
	#endif
		for (int i=0;i<allSize;i++) {
			NarratorNodeIfc & n= (*all_nodes[i]);
			if (&n==NULL) {
				streamOut<<-1;
			} else if (n.isActualNode()||(n.isGroupNode() && &n.getCorrespondingNarratorNode()!=NULL && n.getCorrespondingNarratorNode().isChainNode())) {
				//TODO: check why do we need the check that corresponding node is NULL, must not be the case
				int eq=allocateSerializationNodeEquivalent(all_nodes[i]);
				assert(eq==i);
				streamOut<<eq;
				all_nodes[i]->serialize(streamOut,*this);
			}
		#ifdef PROGRESS_SERIALZATION
			prg->report(i/total*100+0.5);
		#endif
		}
		streamOut<<SERIALIZE_STOP;

		size=top_nodes.size();
		streamOut<<size;
		for (int i=0;i<size;i++) {
			int eq=getSerializationNodeEquivalent(top_nodes[i]);;
			streamOut<<eq;
		#ifdef PROGRESS_SERIALZATION
			prg->report((i+allSize)/total*100+0.5);
		#endif
		}
		hash.serialize(streamOut);
	#ifdef PROGRESS_SERIALZATION
		prg->setCurrentAction("Completed");
		prg->report(100);
	#endif
	}
	NarratorGraph(QDataStream & streamIn,ATMProgressIFC * prg):hash(this) { //equivalent of deserialize
		built=true; //since all nodes are already built but we are reading them
		this->prg=prg;
		colorGuard.setGraph(this);
		all_nodes.clear();
		nodesCount=1;
		all_nodes.append(NULL);

		int size;
		streamIn>>size;
		for (int i=0;i<size;i++) {
			HadithFileDetails v(streamIn);
			hadithFileList.append(v);
			QFile input(v.fileName);
			if (!input.open(QIODevice::ReadOnly)) {
				out << "Hadith File needed but not found: "<<v.fileName<<"\n";
				return;
			}
			QTextStream file(&input);
			file.setCodec("utf-8");
			QString *text=new QString(file.readAll());
			setHadithStringDeserializationIntEquivalent(i,text);
			setHadithStringSerializationEquivalent(text,i);
		}
	#ifdef PROGRESS_SERIALZATION
		int total;
		streamIn>>total;
		prg->setCurrentAction("De-Serializing");
		prg->report(0);
		int counter=0;
	#endif
		if (total>0) {
			int n;
			streamIn>>n;
			while(n!=SERIALIZE_STOP) {
				int cInt=n;
				if (cInt>0) {
					NarratorNodeIfc * node=NarratorNodeIfc::deserialize(streamIn,*this);
					setDeserializationIntEquivalent(cInt,node);
				}
			#ifdef PROGRESS_SERIALZATION
				counter++;
				prg->report(counter/total*100+0.5);
			#endif

				streamIn>>n; //get next number
			}


			streamIn>>size;
			for (int i=0;i<size;i++) {
				int cInt;
				streamIn>>cInt;
				NarratorNodeIfc *n=getDeserializationIntEquivalent(cInt);
				assert(n!=NULL);
				top_nodes.append(n);

			#ifdef PROGRESS_SERIALZATION
				counter++;
				prg->report(counter/total*100+0.5);
			#endif
			}
		}

		hash.deserialize(streamIn);
	#ifdef PROGRESS_SERIALZATION
		prg->setCurrentAction("Completed");
		prg->report(100);
	#endif
		//printChains();
		//printAllNodesList();
		//checkAllGroupHaveCorresponding();
		//qDebug()<<"\n";
	#undef SERIALIZE_STOP
	}
	void fillChainContainer() {
		for (int i=0;i<top_nodes.size();i++) {
			NodeIterator itr=top_nodes[i]->begin();
			for (;!itr.isFinished();++itr) {
				ChainNarratorNode * chain=dynamic_cast<ChainNarratorNode*>(itr.getNode());
				while (!chain->isFirst())
					chain=&chain->prevInChain();
				int chainNumber=chain->getChainNum();
				int oldSize=topChainNodes.size();
				for (int i=oldSize-1;i<chainNumber;i++) {
					topChainNodes.append(NULL);
				}
				topChainNodes[chainNumber]=chain;
			}
		}
		QList<int> nullChains;
		for (int i=0;i<topChainNodes.size();i++) {
			if (topChainNodes[i]==NULL)
				nullChains.append(i);
		}
	#if 0
		for (int i=0;i<all_nodes.size();i++) {
			NarratorNodeIfc * node=all_nodes[i];
			if (node!=NULL && node->isChainNode()) {
				ChainNarratorNode * cNode=dynamic_cast<ChainNarratorNode*>(node);
				int chainNum=cNode->getChainNum();
				if (nullChains.contains(chainNum) ) {
					if(cNode->isFirst()) {
						topChainNodes[chainNum]=cNode;
						nullChains.removeOne(chainNum);
					}
				}
				if (chainNum>=topChainNodes.size() && cNode->isFirst()) {
					int oldSize=topChainNodes.size();
					for (int i=oldSize-1;i<chainNum;i++) {
						topChainNodes.append(NULL);
						nullChains.append(i);
					}
					topChainNodes[chainNum]=cNode;
					nullChains.removeOne(chainNum);
				}
			}
		}
	#endif
	}

	ChainNarratorNode * getChainNode(int chain_num, int narrator_num) {
		assert(chain_num<topChainNodes.size());
		ChainNarratorNode * chain=topChainNodes[chain_num];
		int narrCount=0;
		do {
			if (narrCount==narrator_num)
				return chain;
			if (chain->isLast())
				return NULL;
			chain=&chain->nextInChain();
			narrCount++;
		}while(true);
		return NULL;
	}

	bool equalGraphs(NarratorGraph * other) {
		for (int i=0;i<top_nodes.size();i++) {
			NarratorNodeIfc * n1=top_nodes[i];
			NarratorNodeIfc * n2=findEquivalent(n1,other->top_nodes);
			if (n2==NULL)
				return false;
			if (!equalHelper(n1,n2))
				return false;
		}
		for (int i=0;i<other->top_nodes.size();i++) {
			NarratorNodeIfc * n1=other->top_nodes[i];
			NarratorNodeIfc * n2=findEquivalent(n1,top_nodes);
			if (n2==NULL)
				return false;
			if (!equalHelper(n1,n2))
				return false;
		}
		return true;
	}
	NarratorNodeIfc * getNode (int i) const {
		assert(i>=0 && i<all_nodes.size());
		return all_nodes[i];
	}
	int size() const {
		return all_nodes.size();
	}
	bool isBuilt() { return built; }
	~NarratorGraph() {
		Int2StringMap::iterator i=int2HadithMap.begin();
		for (;i!=int2HadithMap.end();i++)
			delete *i;
		int allSize=all_nodes.size();
		for (int i=0;i<allSize;i++) {
			if (all_nodes[i]!=NULL) {
				delete all_nodes[i];
			}
		}
	}
};

inline int test_GraphFunctionalities(ChainsContainer &chains, ATMProgressIFC *prg,QString fileName)
{
#if 0
	NarratorGraph graph(chains,prg);
#ifdef TEST_BIOGRAPHIES
	QString fileName=getFileName(NULL);
	BiographyList * bioList=getBiographies(fileName,prg);
	for (int i=0;i<bioList->size();i++) {
		for (int j=0;j<bioList->at(i)->size();j++) {
			Narrator * n=(*bioList->at(i))[j];
			ChainNarratorNode * c=graph.getNodeMatching(*n);
			if (c!=NULL)
				c->addBiographyIndex(i);
		}
	}
#endif
	prg->setCurrentAction("Display Graph");
	prg->report(0);
#ifndef TEST_BIOGRAPHIES
	DisplayNodeVisitor visitor;
#else
	bool ok;
	int num = QInputDialog::getInt(NULL, QString("Enter Number of biography to highlight"),
										  QString("User name:"),0,0,chains.size()-1,1,&ok/*,QLineEdit::Normal*/);
	if (!ok)
		num=0;
	DisplayNodeVisitorColored visitor(num);
#endif
	GraphVisitorController c(&visitor,&graph);
	graph.DFS_traverse(c);
	prg->setCurrentAction("Completed");
	prg->report(100);
#else
	NarratorGraph *graph=new NarratorGraph(chains,prg);
	QString * text=NULL;
	if (chains.size()>0) {
		text=chains[0]->hadith_text;
		graph->setFileName(text,fileName);
	}
	QString allName=fileName;
#if !defined(SUBMISSION) || defined(WRITE_POR)
	QFile file(fileName.remove(".txt")+".por");
	file.remove();
	if (!file.open(QIODevice::ReadWrite))
		return -1;
	QDataStream fileStream(&file);
	graph->serialize(fileStream);
	file.close();
#endif
	prg->setCurrentAction("Completed");
	prg->report(100);
	calculateStatisticsOrAnotate(chains,graph,text,allName);
#if 0
	if (!file.open(QIODevice::ReadOnly))
		return -1;
	QDataStream fileStream1(&file);
	NarratorGraph * graph2=new NarratorGraph(fileStream1,prg);
	file.close();
	out<<"equal="<<graph->equalGraphs(graph2)<<"\n";
#endif
#if 0
	delete graph;
#else
	biographies(graph);
#endif
#endif
	return 0;
}

int deserializeGraph(QString fileName,ATMProgressIFC * prg);
int mergeGraphs(QString file1,QString file2,ATMProgressIFC * prg);

#endif // GRAPH_H
