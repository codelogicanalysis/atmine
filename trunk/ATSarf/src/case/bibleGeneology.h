#ifndef BIBLEGENEOLOGY_H
#define BIBLEGENEOLOGY_H

#include <QString>
#include <QDebug>
#include <QFile>
#include "common.h"
#include "text_handling.h"
#include "ATMProgressIFC.h"


#define GET_WAW
//#define TRUST_OLD
#define SINGULAR_DESCENT
#define GENEOLOGYDEBUG
//#define DISPLAY_INDIVIDUAL
#define REDUCE_AFFIX_SEARCH

class GeneologyParameters {
public:
	unsigned int theta_0:10;
	unsigned int N_min:8;
	unsigned int C_max:8;
	unsigned int radius:8;
	GeneologyParameters() {
		theta_0=35;
		N_min=3;
		C_max=3;
		radius=3;
	}
};

extern GeneologyParameters geneologyParameters;

int genealogyHelper(QString input_str,ATMProgressIFC *prgs);
void geneology_initialize();

class GeneNode;
class FillTextVisitor;
class Name {
private:
	friend class FillTextVisitor;
	friend QDataStream &operator>>(QDataStream &in, Name &t);
	friend QDataStream &operator<<(QDataStream &out, const Name &t);
	long start,end;
	QString * text;
private:
	friend QDataStream &operator>>(QDataStream &in, GeneNode &t);
	Name() {}
public:
	Name(QString * text,long start, long end) {
		this->start=start;
		this->end=end;
		this->text=text;
	}
	Name operator=(const Name & n2) {
		text=n2.text;
		start=n2.start;
		end=n2.end;
		return *this;
	}
	QString getString() const {
		return text->mid(start,end-start+1);
	}
	long getStart() const{return start;}
	long getLength() const{return end-start+1;}
	bool operator<(const Name & n) const {
		return getString()<n.getString();
	}
};
class GeneTree;
class GeneNode {
private:
	friend class GeneTree;
	void setParentHeight(int height) {
		if (this==parent) {
			qDebug() << parent->toString();
			return;
		}
		if (parent!=NULL) {
			parent->height=max(parent->height,height);
			parent->setParentHeight(parent->height+1);
		}
	}
	void deleteSubTree() {
		if (this==NULL)
			return;
		for (int i=0;i<children.size();i++) {
			children[i]->deleteSubTree();
		}
		delete this;
	}
	int getSubTreeCount(bool countSpouses=false) {
		if (this==NULL)
			return 0;
		int count=1;
		for (int i=0;i<children.size();i++) {
			count+=children[i]->getSubTreeCount(countSpouses);
		}
		if (countSpouses) {
			count+=spouses.size();
		}
		return count;
	}
	GeneNode * getNodeInSubTree(QString word,bool checkSpouses=false, int maxDepth=-1) {
		if (this==NULL)
			return NULL;
		if (!ignoreInSearch && equal_withoutLastDiacritics(word,name.getString()))
			return this;
		if (checkSpouses) {
			for (int i=0;i<spouses.size();i++) {
				if (equal_withoutLastDiacritics(word,spouses[i].getString()))
					return this;
			}
		}
		if (maxDepth<0 || maxDepth>0) {
			int new_maxDepth=(maxDepth<0 ?maxDepth: maxDepth-1);
			for (int i=0;i<children.size();i++) {
				GeneNode * found=children[i]->getNodeInSubTree(word,checkSpouses,new_maxDepth);
				if (found!=NULL)
					return found;
			}
		}
		return NULL;
	}
	void printTree(int indentation) {
		if (this==NULL)
			out<<"NULL";
		else {
			QString sp="";
			if (spouses.size()>0) {
				sp+="[";
				for (int i=0;i<spouses.size();i++) {
					sp+=(i!=0?"\t":"")+spouses[i].getString();
				}
				sp+="] ";
			}
			out<<QString(indentation,'>')/*<<"["<<height<<"]"*/<<name.getString()<<sp<<"\n";
			for (int i=0;i<children.size();i++) {
				children[i]->printTree(indentation+1);
			}
		}
	}

	friend QDataStream &operator>>(QDataStream &in, GeneNode &t);
	friend QDataStream &operator>>(QDataStream &in, GeneTree &t);
	GeneNode(): name(NULL,-1,-1) {}

public:
	Name name;
	GeneNode * parent;
	QList<Name> spouses;
	QList<GeneNode *> children;
	int height;
	bool ignoreInSearch;

	GeneNode(Name n, GeneNode * parent ):name(n) {
		assert(parent!=this);
		this->parent=parent;
		height=0; //since no children
		setParentHeight(1);
		if (parent!=NULL)
			parent->addChild(this);
		ignoreInSearch=false;
	}
	GeneNode * addChild(GeneNode * n) { //return itself
		if (this!=NULL) {
			if (n!=NULL) {
				children.append(n);
				n->parent=this;
				n->setParentHeight(n->height+1);
			}
			return this;
		} else
			return n;
	}
	GeneNode * addParent(GeneNode * n) { //return new parent
		if (this!=NULL) {
			if (n!=NULL) {
				if (parent!=NULL) {
					parent->addSpouse(n->name);
					for (int i=0;i<n->spouses.size();i++)
						parent->addSpouse(spouses[i]);
					return parent;
				} else {
					n->addChild(this);
					return n;
				}
			}
			return this;
		} else
			return n;
	}
	bool hasSpouse(const Name & n) const {
		if (this==NULL)
			return false;
		for (int i=0;i<spouses.size();i++) {
			if (equal_withoutLastDiacritics(n.getString(),spouses[i].getString()))
				return true;
		}
		return false;
	}
	bool hasChild(const Name & n) const {
		if (this==NULL)
			return false;
		for (int i=0;i<children.size();i++) {
			if (equal_withoutLastDiacritics(n.getString(),children[i]->toString()))
				return true;
		}
		return false;
	}
	GeneNode * addSpouse(const Name & n) { //return itself
		if (this!=NULL) {
			spouses.append(n);
		}
		return this;
	}
	GeneNode * addSibling(GeneNode * n) { //return itself
		if (this!=NULL) {
			if (parent!=NULL) {
				parent->children.append(n);
				if (n!=NULL) {
					n->parent=parent;
					parent->setParentHeight(n->height+1);
				}
			}
			return this;
		} else
			return n;
	}
	bool isLeaf() {
		return children.size()==0;
	}
	QString toString() const {
		if (this==NULL)
			return "NULL";
		else
			return name.getString();
	}
};
class GeneTree {
private:
	friend QDataStream &operator<<(QDataStream &out, const GeneTree &t);
	friend QDataStream &operator>>(QDataStream &in, GeneTree &t);
	class MergeVisitor;
	MergeVisitor * mergeVisitor;
	GeneNode * root;
public:
	GeneTree() {
		root=NULL;
		mergeVisitor=NULL;
	}
	GeneTree(GeneNode * root) {
		this->root=root;
		mergeVisitor=NULL;
	}
	void setRoot(GeneNode * node) {
		root=node;
	}
	GeneNode * getRoot() {
		if (this==NULL)
			return NULL;
		return root;
	}
	void deleteTree() {
		if (this==NULL)
			return;
		//out<<"\n{deleting}\n";
		//root->printTree(0);
		//root->deleteSubTree();
		delete this;
	}
	void updateRoot() {
		if (this==NULL)
			return;
		if (root==NULL)
			return;
		while (root->parent!=NULL)
			root=root->parent;
	}
	void outputTree() {
		//out<<"{Output}\n";
		if (this==NULL)
			out<<"NULL";
		else
			root->printTree(0);
		out<<"\n";
	}
	int getTreeLevels() {
		if (this==NULL)
			return 0;
		if (root==NULL)
			return 0;
		return root->height+1;
	}
	int getTreeNodesCount(bool countSpouses=false) {
		if (this==NULL)
			return 0;
		return root->getSubTreeCount(countSpouses);
	}
	GeneNode * findTreeNode(QString word, bool checkSpouses=false) {
		if (this==NULL)
			return NULL;
		return root->getNodeInSubTree(word,checkSpouses);
	}
	void compareToStandardTree(GeneTree * standard);
	void mergeTrees(GeneTree * tree);
	void mergeLeftovers();
	void displayTree( ATMProgressIFC * prg);
	~GeneTree();
};
class GeneVisitor {
private:
	void visit(const GeneNode * node) {
		if (node==NULL)
			return;
		visit(node,node->height);
		for (int i=0;i<node->spouses.size();i++) {
			visit(node,node->spouses[i],true);
			//visit(node->spouses[i],node->height);
		}
		for (int i=0;i<node->children.size();i++) {
			if (node->children[i]!=NULL)
				visit(node,node->children[i]->name,false);
			visit(node->children[i]);

		}

	}
public:
	virtual void visit(const GeneNode * node, int height)=0;
	virtual void visit(const GeneNode * node1, const Name & name2, bool isSpouse)=0;
	virtual void finish()=0;
	void operator ()(GeneTree * tree) {
		GeneNode * root=tree->getRoot();
		visit(root);
		finish();
	}
};
class GeneDisplayVisitor: public GeneVisitor {
protected:
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
	QList<QStringList> ranksList;
	QSet<long> set;

	long getUniqueNodeID(const Name & n,bool isSpouse) {//if not there returns -1
		long id=(long)&n;
		if (!set.contains(id)) {
			d_out<<QString("n%1 [label=\"%2\" %3]\n").arg(id).arg(n.getString()).arg((isSpouse?",style=filled, fillcolor=grey":""));
		}
		return id;
	}
	void setGraphRank(int rank, QString s)	{
		while(rank>=ranksList.size())
			ranksList.append(QStringList());
		ranksList[rank].append(s);
	}
	QString getAndInitializeDotNode(const Name & n,bool isSpouse) {
		long curr_id=getUniqueNodeID(n,isSpouse);
		return QString("n%1").arg(curr_id);
	}
	virtual void initialize() {
		ranksList.clear();
		file=new QFile("graph.dot");
		file->remove();
		if (!file->open(QIODevice::ReadWrite)) {
			out<<"Error openning file\n";
			return;
		}

		dot_out=new QTextStream(file);
		d_out.setCodec("utf-8");
		d_out<<"digraph gene_graph {\n";
	}
	virtual void visit(const GeneNode * n1,const Name & n2, bool isSpouse)	{
		QString s1=getAndInitializeDotNode(n1->name,false), s2=getAndInitializeDotNode(n2,isSpouse);
		d_out<<s1<<"->"<<s2<<" ;\n";
	}
	virtual void visit(const GeneNode * n, int) {
		getAndInitializeDotNode(n->name,false);
	}
	virtual void finish() {
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
public:
	GeneDisplayVisitor(){
		initialize();
	}
};

inline QDataStream &operator<<(QDataStream &out, const Name &t) {
	out<<(qint32)t.start<<(qint32)t.end;
	return out;
}
inline QDataStream &operator>>(QDataStream &in, Name &t) {
	qint32 l,e;
	in >>l>>e;
	t.start=l;
	t.end=e;
	t.text=NULL;
	return in;
}
inline QDataStream &operator<<(QDataStream &out, const GeneNode &t) {
	if (&t!=NULL) {
		out <<false
			<<t.name
			<<t.height
			<<t.ignoreInSearch
			<<t.children.size();
		for (int i=0;i<t.children.size();i++) {
			out<<*(t.children[i]);
		}
		out<<t.spouses.size();
		for (int i=0;i<t.spouses.size();i++) {
			out<<t.spouses[i];
		}
	} else {
		out<<true;
	}
	return out;
}
inline QDataStream &operator>>(QDataStream &in, GeneNode &t) {
	int childrenCount,spouseCount;
	in  >>t.name
		>>t.height
		>>t.ignoreInSearch
		>>childrenCount;
	t.parent=NULL;
	for (int i=0;i<childrenCount;i++) {
		bool isNull;
		in >>isNull;
		assert(!isNull);
		GeneNode * n=new GeneNode();
		in >>*n;
		t.addChild(n);
	}
	in  >>spouseCount;
	for (int i=0;i<spouseCount;i++) {
		Name n;
		in >>n;
		t.spouses.append(n);
	}
	return in;
}
inline QDataStream &operator<<(QDataStream &out, const GeneTree &t) {
	out<<*t.root;
	return out;
}
inline QDataStream &operator>>(QDataStream &in, GeneTree &t) {
	bool isNull;
	in >>isNull;
	if (!isNull) {
		t.root=new GeneNode();
		in >>*t.root;
	} else {
		t.root=NULL;
	}
	return in;
}


#endif // BIBLEGENEOLOGY_H