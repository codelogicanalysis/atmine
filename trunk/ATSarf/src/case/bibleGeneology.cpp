#include <QTextBrowser>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <assert.h>

#include "bibleManualTagger.h"
#include "bibleGeneology.h"
#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stemmer.h"
#include "letters.h"
#include "diacritics.h"
#include "text_handling.h"
#include "textParsing.h"
#include "common.h"
#include "Retrieve_Template.h"


#define DETAILED_DISPLAY

enum WordType { NEW_NAME,CORE_NAME, LEAF_NAME, DC,ENDING_PUNC,PARA_PUNC,COLON_PUNC, OTHER};
enum DescentDirection {SPOUSE, SON, FATHER, SIBLING,UNDEFINED_DIRECTION};
enum StateType { TEXT_S , NAME_S, SONS_S};

#ifdef GENEOLOGYDEBUG
inline QString type_to_text(WordType t) {
	switch(t){
		case NEW_NAME:
			return "NEW_NAME";
		case LEAF_NAME:
			return "LEAF_NAME";
		case CORE_NAME:
			return "CORE_NAME";
		case DC:
			return "DC";
		case OTHER:
			return "OTHER";
		case ENDING_PUNC:
			return "ENDING_PUNC";
		case PARA_PUNC:
			return "PARA_PUNC";
		case COLON_PUNC:
				return "COLON_PUNC";
		default:
			return "UNDEFINED-TYPE";
	}
}
inline QString type_to_text(StateType t) {
	switch(t){
		case TEXT_S:
			return "TEXT_S";
		case NAME_S:
			return "NAME_S";
		case SONS_S:
			return "SONS_S";
		default:
			return "UNDEFINED-TYPE";
	}
}
inline QString type_to_text(DescentDirection t) {
	switch(t){
		case SPOUSE:
			return "SPOUSE";
		case SON:
			return "SON";
		case FATHER:
			return "FATHER";
		case SIBLING:
			return "SIBLING";
		case UNDEFINED_DIRECTION:
			return "UNDEFINED_DIRECTION";
		default:
			return "UNDEFINED";
	}
}
inline void display(WordType t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t)<<" ";
}
inline void display(StateType t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t);
}
inline void display(DescentDirection t) {
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t);
}
inline void display(QString t) {
	out<<t;
	//qDebug() <<t;
}
#else
	#define display(c)
#endif

class DescentConnectors;
class DescentConnectorGroup {
private:
	friend class DescentConnectors;
	QList<long> descriptions;
	DescentDirection dir;
	bool plural;
	long operator[](int i) { return descriptions[i]; }
	int size() { return descriptions.size(); }
public:
	DescentConnectorGroup(DescentDirection dir, bool plural=false) {
		this->dir=dir;
		this->plural=plural;
	}
	DescentConnectorGroup() {
		this->dir=UNDEFINED_DIRECTION;
	}
	void addDescription(long description_id) {
		descriptions<<description_id;
	}
	bool isDescription(long description_id) {
		for (int j=0;j<size();j++) {
			if (descriptions[j]==description_id)
				return true;
		}
		return false;
	}
};
class DescentConnectors {
private:
	QList<DescentConnectorGroup> groups;
public:
	void addDescentGroup(DescentConnectorGroup group) {
		groups<<group;
	}
	DescentDirection getDirection(long description_id, bool & plural) {
		for (int i=0;i<groups.size();i++) {
			if (groups[i].isDescription(description_id)) {
				plural=groups[i].plural;
				return groups[i].dir;
			}
		}
		plural=false;
		return UNDEFINED_DIRECTION;
	}
	DescentDirection getDirection(long description_id) {
		bool plural;
		return getDirection(description_id,plural);
	}
};


GeneologyParameters geneologyParameters;
QList<int> bits_gene_NAME;
QStringList relativeSuffixes;
int bit_VERB_PERFECT, bit_LAND, bit_FEMALE,bit_MALE;
DescentConnectors descentConnectors;
DescentConnectorGroup landDesc;

class GeneStemmer: public Stemmer {
private:
	bool place;
public:
	long finish_pos;
	bool name:1;
	bool male:1;
	bool gaveBirth:1;
	bool land:1;
	DescentDirection descentDir:3;
	bool pluralDescent:1;
	bool relativeSuffix;
#ifdef GET_WAW
	bool has_waw:1;
	bool has_li:1;
#endif
	long startStem, finishStem,wawStart,wawEnd;

	GeneStemmer(QString * word, int start)
		:Stemmer(word,start,false)	{
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start) {
		this->info.start=start;
		this->info.finish=start;
		name=false;
		descentDir=UNDEFINED_DIRECTION;
		pluralDescent=false;
		gaveBirth=false;
		land=false;
		finish_pos=start;
		startStem=start;
		finishStem=start;
		relativeSuffix=false;
		male=true;
	#ifdef GET_WAW
		wawStart=start;
		wawEnd=start;
		has_waw=false;
		has_li=false;
	#endif
	}
	bool on_match()	{
		solution_position * S_inf=Stem->computeFirstSolution();
		do {
			stem_info=Stem->solution;
		#ifdef GET_AFFIXES_ALSO
			solution_position * p_inf=Prefix->computeFirstSolution();
			do
			{
				prefix_infos=&Prefix->affix_info;
				solution_position * s_inf=Suffix->computeFirstSolution();
				do
				{
					suffix_infos=&Suffix->affix_info;
		#endif
					if (!analyze())
						return false;
		#ifdef GET_AFFIXES_ALSO
				}while (Suffix->computeNextSolution(s_inf));
				delete s_inf;
			}while (Prefix->computeNextSolution(p_inf));
			delete p_inf;
		#endif
		}while (Stem->computeNextSolution(S_inf));
		delete S_inf;
		return true;
	}

#ifdef GET_WAW
	void checkForWaw() {
		has_waw=false;
		has_li=false;
	#ifndef GET_AFFIXES_ALSO
		solution_position * p_inf=Prefix->computeFirstSolution();
		do {
			prefix_infos=&Prefix->affix_info;
	#endif
		#ifndef REDUCE_AFFIX_SEARCH
			for (int i=0;i<prefix_infos->size();i++) {
		#else
			if (prefix_infos->size()>0) {
				int i=0;
		#endif
				has_waw=prefix_infos->at(i).POS=="wa/CONJ+";
				if (!has_waw)
					has_li=prefix_infos->at(i).POS=="li/PREP+";
				if (has_waw || has_li) {
					wawStart=(i==0?Prefix->info.start:Prefix->getSplitPositions().at(i-1)-1);
					wawEnd=Prefix->getSplitPositions().at(i)-1;
					break;
				}
			}
			if (has_waw || has_li)
				break;
	#ifndef GET_AFFIXES_ALSO
		}while (Prefix->computeNextSolution(p_inf));
		delete p_inf;
	#endif
	}
#endif

	bool analyze()	{
		if (info.finish>info.start) { //more than one letter to be tested for being a name
			if (landDesc.isDescription(stem_info->description_id)) {
				land=true;
				return false;
			}
			bool plural;
			descentDir=descentConnectors.getDirection(stem_info->description_id,plural);
			if (descentDir!=UNDEFINED_DIRECTION) {
				pluralDescent=plural;
				int suffSize=Suffix->info.finish - Suffix->info.start+1;
				gaveBirth=stem_info->abstract_categories.getBit(bit_VERB_PERFECT);
				if (suffSize>0 && !gaveBirth ) {
					QString suffix=info.text->mid(Suffix->info.start,suffSize);
					for (int i=0;i<relativeSuffixes.size();i++) {
						relativeSuffix=true;
						if (equalNames(suffix,relativeSuffixes[i])) {
							if (descentDir==SON) {
								descentDir=FATHER;
								pluralDescent=true;
							} else if (descentDir==FATHER) {
								descentDir=SON;
								pluralDescent=true;
							}
							break;
						}
					}
				}
				return false;
			}
			int bitsNamesSize=bits_gene_NAME.size();
			for (int i=0;i<bitsNamesSize;i++) {
				if (stem_info->abstract_categories.getBit(bits_gene_NAME[i]))
						//&& Suffix->info.finish-Suffix->info.start<0 && Stem->info.finish>Stem->info.start) //i.e. has no suffix and stem > a letter
				{
					name=true;
					if (info.finish>finish_pos) {
						finish_pos=info.finish;
						finishStem=Stem->info.finish;
						startStem=Stem->info.start;
						if (stem_info->abstract_categories.getBit(bit_FEMALE))
							male=false;
						else if (stem_info->abstract_categories.getBit(bit_MALE))
							male=true;
					}
				#ifdef GET_WAW
					if (info.finish==finish_pos && !has_waw && !has_li)
						checkForWaw();
				#endif
					return true;
				}
			}
		}
		return true;
	}
};

void geneology_initialize() {
	long abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Hebrew Bible Names");
	int bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	bits_gene_NAME.append(bit_NAME);
#if 0
	abstract_NAME=database_info.comp_rules->getAbstractCategoryID("christian names");
	bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	bits_gene_NAME.append(bit_NAME);

	abstract_NAME=database_info.comp_rules->getAbstractCategoryID("Male Names");
	bit_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NAME);
	bits_gene_NAME.append(bit_NAME);
#endif

	DescentConnectorGroup spouse(SPOUSE);
	Retrieve_Template d_spouse("description","id","name='woman' OR name LIKE '%spouse%' OR name='concubine'");
	while (d_spouse.retrieve())
		spouse.addDescription(d_spouse.get(0).toULongLong());
	DescentConnectorGroup spouses(SPOUSE,true);
	Retrieve_Template d_spouses("description","id","name='women' OR name='concubines'");
	while (d_spouses.retrieve())
		spouses.addDescription(d_spouses.get(0).toULongLong());
	DescentConnectorGroup sibling(SIBLING);
	Retrieve_Template d_sibling("description","id","name='brother' OR name='sister' OR name='sister/counterpart'");
	while (d_sibling.retrieve())
		sibling.addDescription(d_sibling.get(0).toULongLong());
	DescentConnectorGroup siblings(SIBLING,true);
	Retrieve_Template d_siblings("description","id","name='brothers' OR name='sisters'");
	while (d_siblings.retrieve())
		siblings.addDescription(d_siblings.get(0).toULongLong());
	DescentConnectorGroup father(FATHER);
	Retrieve_Template d_father("description","id","name='son' OR name='daughter/girl'");
	while (d_father.retrieve())
		father.addDescription(d_father.get(0).toULongLong());
	DescentConnectorGroup son(SON);
	Retrieve_Template d_son("description","id","name='father' OR name='mother/maternal' OR name='mother'");
	while (d_son.retrieve())
		son.addDescription(d_son.get(0).toULongLong());
	DescentConnectorGroup sons(SON,true);
	Retrieve_Template d_sons("description","id","name='give birth to' OR name='be born' OR name LIKE 'sons/children%' OR name='daughters/girls'");
	while (d_sons.retrieve())
		sons.addDescription(d_sons.get(0).toULongLong());
	descentConnectors.addDescentGroup(spouse);
	descentConnectors.addDescentGroup(sibling);
	descentConnectors.addDescentGroup(father);
	descentConnectors.addDescentGroup(son);
	descentConnectors.addDescentGroup(sons);
	descentConnectors.addDescentGroup(siblings);
	descentConnectors.addDescentGroup(spouses);

	Retrieve_Template d_land("description","id","name='earth/territory' OR name LIKE 'feddan%'");
	while (d_land.retrieve())
		landDesc.addDescription(d_land.get(0).toULongLong());

	//abou ... bani and kharaja minho/minhoma/minhom => non direct descent

	long abstract_VERB_PERFECT=database_info.comp_rules->getAbstractCategoryID("VERB_PERFECT");
	bit_VERB_PERFECT=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_VERB_PERFECT);
	long abstract_FEMALE=database_info.comp_rules->getAbstractCategoryID("Female Names");
	bit_FEMALE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_FEMALE);
	long abstract_MALE=database_info.comp_rules->getAbstractCategoryID("Male Names");
	bit_MALE=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_MALE);

	relativeSuffixes<<QString("")+ha2<<QString("")+ha2+alef<<QString("")+ha2+meem+alef<<QString("")+ta2+ha2<<QString("")+ta2+ha2+alef<<QString("")+ta2+ha2+meem+alef;
}

class GeneTagVisitor: public GeneVisitor {
private:
	ATMProgressIFC *prg;
public:
	GeneTagVisitor(ATMProgressIFC *prg) {
		this->prg=prg;
	}
	void visit(const GeneNode * node, int) {
		const Name & name=node->name;
		prg->tag(name.getStart(),name.getLength(),Qt::white,true);
	}
	void visit(const GeneNode * ,const Name &,bool) {}
	void finish() {}
};
class GeneTree::MergeVisitor:public GeneVisitor{
private:
	typedef QPair<Name,Name> Edge;
	typedef QMap<Edge,bool> EdgeMap;
private:
	GeneTree * mainTree;
	GeneNode * topMostNode;
	EdgeMap unPerformedEdges;
	bool newTree;
	void addUnPerformedEdge(const Name & n1,const Name & n2, bool isSpouse=false) {
		unPerformedEdges[Edge(n1,n2)]=isSpouse;
	}
	GeneNode * find(const Name & name, bool checkSpouses=false) {
		if (topMostNode==NULL) {
			GeneNode * node=mainTree->findTreeNode(name.getString(),checkSpouses);
			if (node!=NULL)
				topMostNode=node;
			return node;
		} else {
			GeneNode * node= topMostNode->getNodeInSubTree(name.getString(),checkSpouses);
		#if 0
			if (node==NULL) {
				node=mainTree->findTreeNode(name.getString(),checkSpouses);
			}
		#endif
			return node;
		}
	}
	bool notFoundWithinRadius(GeneNode * node, const Name & searchedName, bool searchSpouses=false) {
		int radius=geneologyParameters.radius;
		GeneNode * temp=node->parent;
		int tempRadius=radius;
		for (int i=0;i<radius;i++) {
			if (temp!=NULL) {
				tempRadius++;
				node->ignoreInSearch=true;
				GeneNode *temp2=temp->getNodeInSubTree(searchedName.getString(),searchSpouses,tempRadius);
				node->ignoreInSearch=false;
				if (temp2!=NULL)
					return false;

				if (equalNames(temp->toString(),searchedName.getString()))
					return false;
				for (int j=0;j<temp->spouses.size();j++) {
					if (equalNames(temp->spouses[j].getString(),searchedName.getString()))
						return false;
				}
				temp=temp->parent;
			}
		}
		node->ignoreInSearch=true;
		temp=node->getNodeInSubTree(searchedName.getString(),searchSpouses,radius);
		node->ignoreInSearch=false;
		if (temp!=NULL)
			return false;
		return true;
	}
	void performAction(const Name & name1, const Name & name2, bool isSpouse) {
		GeneNode * n1=find(name1,true);
		GeneNode * n2=find(name2,true);
		if (newTree && n1!=NULL && n2 !=NULL && notFoundWithinRadius(n1,name2)) {
			topMostNode=n1;
			n2=NULL;
		}
		if (isSpouse) {
			if (n1==n2) {
				if (n1!=NULL) {
					//correctly placed
				} else {
					addUnPerformedEdge(name1,name2,isSpouse);
				}
			} else {
				if (n1==NULL) {
					if (notFoundWithinRadius(n2,name1,true))
						n2->addSpouse(name1);
				} else if (n2==NULL) {
					if (notFoundWithinRadius(n1,name2,true))
						n1->addSpouse(name2);
				} else {
				#if 0
					//try merging spouses 2 nodes if possible
					GeneNode * main=n1;
					if (n1->ignoreInSearch) {
						bool found=false;
						for (int j=0;j<2;j++) {
							GeneNode * s=(j==0?n1:n2);
							for (int i=0;i<s->spouses.size();i++) {
								GeneNode * n=mainTree->findTreeNode(s->spouses[i].getString());
								if (n!=NULL && !n->ignoreInSearch) {
									main=n;
									found=true;
									break;
								}
							}
							if (found)
								break;
						}
						assert(found);
					}
					for (int i=0;i<n2->children.size();i++) {
						n2->children[i]->parent=NULL;
						main->addChild(n2->children[i]);
					}
					n2->ignoreInSearch=true;
				#endif
				}
			}
		} else {
			if (n1==n2) {
				if (n1!=NULL) {
				#ifdef SHOW_MERGING_ERRORS
					error<< "Conflict ("<<n1->toString()<<","<<n2->toString()<<") old relationship is spouse, new is child.\n";
					//conflict but trust old (this means the nodes correspond to spouses)
				#endif
				} else {
					addUnPerformedEdge(name1,name2,isSpouse);
				}
			} else {
				if (n1==NULL) {
					if (notFoundWithinRadius(n2,name1,true)) {
						n2->addParent(new GeneNode((Name & )name1,NULL));
						mainTree->updateRoot();
					}
				} else if (n2==NULL) {
					if (notFoundWithinRadius(n1,name2))
						n1->addChild(new GeneNode((Name & )name2,NULL));
				} else {
					if (n2->parent!=n1) {
					#ifdef SHOW_MERGING_ERRORS
						//conflict but trust old
						error<< "Conflict ("<<n1->toString()<<","<<n2->toString()<<") newly must be child relationship but previously is not.\n";
					#endif
					}
				}
			}
		}
	}

public:
	MergeVisitor(GeneTree * tree2){
		this->mainTree=tree2;
		topMostNode=NULL;
	}
	void operator ()(GeneTree * tree) {
		if (tree->getRoot()!=NULL) {
			topMostNode=find(tree->getRoot()->name);
			newTree=true;
			GeneVisitor::operator ()(tree);
			newTree=false;
		}
	}

	virtual void visit(const GeneNode * , int ){	}
	virtual void visit(const GeneNode * node1, const Name & name2, bool isSpouse){
		const Name & name1=node1->name;
		performAction(name1,name2,isSpouse);
	}
	void finish() {}
	void tryPerformingUnperformedNodes() {
		EdgeMap::iterator itr=unPerformedEdges.begin();
		for (;itr!=unPerformedEdges.end();itr++) {
			performAction(itr.key().first,itr.key().second,itr.value());
		}
	}
};
class FindAllVisitor:public GeneVisitor {
private:
	QSet<GeneNode*> & visited;

	const GeneNode * nodeToMatch;
	GeneNode * bestMatch;
	int bestScore;

	GeneNode * currMatch; //temporary
	int currScore; //temporary
private:
	void finalizeMatch() {
		if (currMatch!=NULL && !visited.contains(currMatch)) {
			if (nodeToMatch->parent==NULL) {
				if (currMatch->parent==NULL )
					currScore++;
			} else {
				if (equalNames(nodeToMatch->parent->toString(),currMatch->parent->toString()))
					currScore++;
			}

			if (bestMatch==NULL || bestScore<currScore) {
				bestMatch=currMatch;
				bestScore=currScore;
			}
		}
	}

public:
	FindAllVisitor(const GeneNode * nodeToMatch, QSet<GeneNode*> & visitedNodes):visited(visitedNodes) {
		this->nodeToMatch=nodeToMatch;
		bestMatch=NULL;
		currMatch=NULL;
		bestScore=0;
	}
	virtual void visit(const GeneNode * node, int ) {
		finalizeMatch();
		if (equalNames(node->toString(),nodeToMatch->toString())){
			currMatch= (GeneNode *)node;
		} else {
			currMatch= NULL;
		}
		currScore=0;
	}
	virtual void visit(const GeneNode * node1, const Name & name2, bool isSpouse){
		if (node1==currMatch) {
			if (isSpouse) {
				if (nodeToMatch->hasSpouse(name2))
					bestScore++;
			} else {
				if (nodeToMatch->hasChild(name2))
					bestScore++;
			}
		}
	}
	virtual void finish() {
		finalizeMatch();
		if (bestMatch!=NULL) {
			assert(!visited.contains(bestMatch));
			visited.insert(bestMatch);
		}
	}
	bool isFound() {
		return bestMatch!=NULL;
	}
	GeneNode * getFoundNode() {
		return bestMatch;
	}
	int getFoundScore() {
		return bestScore;
	}
};
class CompareVisitor: public GeneVisitor {
private:
	QSet<GeneNode*> & visitedNodes;
	GeneTree * standard;
	int foundCount, similarContextCount,totalCount, countOfThisTree;
public:
	CompareVisitor(GeneTree * standard,QSet<GeneNode*> & visited):visitedNodes(visited) {
		foundCount=0;
		similarContextCount=0;
		totalCount=0;
		this->standard=standard;
		//standard->updateRoot();
	}
	virtual void visit(const GeneNode * node, int ) {
		if (node->parent==NULL)
			countOfThisTree=node->getSubTreeCount();
		FindAllVisitor v(node,visitedNodes);
		v(standard);
		if (v.isFound()) {
			foundCount++;
			if (v.getFoundScore()>0)
				similarContextCount++;
		}
		totalCount++;
	}
	virtual void visit(const GeneNode *, const Name & , bool ){	}
	virtual void finish() {	}
	double getFoundPercentage() { //TODO: make sure we are dividing by the correct number
		return (double)foundCount/ /*totalCount*/standard->getTreeNodesCount();
	}
	double getSimilarContextPercentage() {
		assert(totalCount==countOfThisTree);
		return (double)similarContextCount/ /*totalCount*/standard->getTreeNodesCount();
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
class DuplicateVisitor:public GeneVisitor {
private:
	GeneTree * tree;
public:
	DuplicateVisitor() {
		tree=new GeneTree();
	}
	virtual void visit(const GeneNode * node,int) {
		if (tree->getRoot()==NULL) {
			GeneNode * n=new GeneNode(node->name,NULL);
			tree->setRoot(n);
		}
	}
	virtual void visit(const GeneNode * node, const Name & name2, bool isSpouse) {
		GeneNode * lastNode = tree->findTreeNode(node->toString(),false);
		if (isSpouse) {
			lastNode->addSpouse(name2);
		} else {
			assert(lastNode!=NULL);
			new GeneNode(name2,lastNode);
		}
	}
	virtual void finish() {}
	GeneTree * getDuplicateTree() { return tree;}
};
GeneTree* GeneTree::duplicateTree() {
	DuplicateVisitor v;
	v(this);
	return v.getDuplicateTree();
}
void GeneTree::compareToStandardTree(GeneTree * standard,QSet<GeneNode*> & visitedNodes,double & found,double & similarContext) {
	CompareVisitor v(standard,visitedNodes);
	v(this);
	found=v.getFoundPercentage();
	similarContext=v.getSimilarContextPercentage();
}
void GeneTree::compareToStandardTree(GeneTree * standard,double & found,double & similarContext) {
	QSet<GeneNode*> visitedNodes;
	CompareVisitor v(standard,visitedNodes);
	v(this);
	found=v.getFoundPercentage();
	similarContext=v.getSimilarContextPercentage();
}
void GeneTree::mergeTrees(GeneTree *tree) {
	if (mergeVisitor==NULL)
		mergeVisitor=new GeneTree::MergeVisitor(this);
	(*mergeVisitor)(tree);
}
void GeneTree::mergeLeftovers() {
	if (mergeVisitor!=NULL)
		mergeVisitor->tryPerformingUnperformedNodes();
}
void GeneTree::displayTree( ATMProgressIFC * prg) {
	if (this!=NULL) {
		GeneDisplayVisitor * d=new GeneDisplayVisitor();
		(*d)(this);
		delete d;
		prg->displayGraph();
	}
}
GeneTree::~GeneTree() {
	if (mergeVisitor!=NULL) {
		delete mergeVisitor;
	}
}

class GenealogySegmentor {
private:
	class StateInfo{
	public:
		QString * text;
		long lastEndPos;
		long startPos;
		long endPos;
		long nextPos;
		WordType currentType:3;
		StateType currentState:2;
		StateType nextState:2;
		bool preceededByWaw:1;
		bool preceededByLi:1;
		bool preceededBygaveBirth:1;
		bool sonsFoundName:1;
		bool land:1;
		bool male:1;
		DescentDirection descentDirection:3;
		DescentDirection lastDescentDirection:3;
		bool singularDescent:1;
		//int unused:19;
		PunctuationInfo previousPunctuationInfo,currentPunctuationInfo;
		void resetCurrentWordInfo()	{
			currentPunctuationInfo.reset();
			preceededByWaw=false;
			preceededByLi=false;
			//descentDirection=UNDEFINED_DIRECTION;
		}
		QString getWord() {	return text->mid(startPos,endPos-startPos+1); }
		void initialize(QString * text){
			resetCurrentWordInfo();
			currentState=TEXT_S;
			nextState=TEXT_S;
			lastEndPos=0;
			startPos=0;
			previousPunctuationInfo.fullstop=true;
			this->text=text;
			preceededByWaw=false;
			preceededByLi=false;
			preceededBygaveBirth=false;
			singularDescent=true;
			lastDescentDirection=UNDEFINED_DIRECTION;
			land=false;
		}
		StateInfo() {
			initialize(NULL);
		}
	};
	typedef BibleTaggerDialog::Selection OutputData;
	typedef BibleTaggerDialog::SelectionList OutputDataList;
	typedef BibleTaggerDialog::Selection::MainSelection Selection;
	typedef BibleTaggerDialog::Selection::MainSelectionList SelectionList;
	OutputDataList outputList;
	class StateData {
	public:
		int i0;
		GeneTree * globalTree;
		GeneTree * tree;
		GeneNode * last;
		QString lastName;
		long startGene;
		OutputData * outputData;
	public:
		StateData() {
			outputData=NULL;
			initialize();
			globalTree=NULL;
		}
		void initialize() {
			if (outputData==NULL) {
				outputData=new OutputData;
			} else {
				outputData->clear();
			}
			i0=0;
			last=NULL;
			tree=NULL;
			startGene=-1;
		}
	};
private:
	QString fileName;
	StateData currentData;
	StateInfo stateInfo;
	QString * text;
	ATMProgressIFC *prg;

	inline bool conditionToOutput() {
		return currentData.tree->getTreeLevels()>=geneologyParameters.N_min ||
				currentData.tree->getTreeNodesCount(true)>=geneologyParameters.C_max;
	}
	inline bool checkIfDisplay(bool keep=false) {
		bool ret_val=true;
		if (!keep) {
			if (conditionToOutput()) {
				outputAndTag();
				ret_val= false;
			} else {
				currentData.tree->deleteTree();
				currentData.outputData->clear();
			}
			currentData.last=NULL;
			currentData.tree=NULL;
			stateInfo.nextState=TEXT_S; //redundant
		}
		return ret_val;
	}
	inline bool doParaCheck() {
		int count=currentData.tree->getTreeNodesCount(true);
		display(QString("{keep? count %1}").arg(count));
		bool keep=count<=geneologyParameters.C_max && count>1;
		bool ret_val=checkIfDisplay(keep);
		if (!keep) {
			currentData.outputData->clear();
			stateInfo.nextState=TEXT_S;
		} else {
			if (conditionToOutput()) {
				outputAndTag();
				currentData.last=NULL;
				currentData.tree=NULL;
				stateInfo.nextState=TEXT_S; //redundant
				ret_val= false;
			} else {
				currentData.last=NULL;
				stateInfo.nextState=NAME_S;
			}
		}
		stateInfo.descentDirection=UNDEFINED_DIRECTION;
		return ret_val;
	}
	inline bool doActionNewNameAndNullLast(Name & name) {
		bool ret_value=true;
	#ifdef TRUST_OLD
		if (currentData.tree->getTreeNodesCount()==0) {
	#endif
			DescentDirection dir=stateInfo.descentDirection;
			ret_value=doParaCheck(); //nameList is cleared here, if needed
			if (currentData.outputData->getNamesList().size()>0) {
				currentData.tree->deleteTree();
				currentData.outputData->clear();
			}
			currentData.outputData->addName(name);
			currentData.startGene=stateInfo.startPos;
			currentData.last=new GeneNode(name,NULL);
			currentData.tree=new GeneTree(currentData.last);
			currentData.i0=0;
			if (dir!=UNDEFINED_DIRECTION) {
				if (dir==SON)
					stateInfo.nextState=SONS_S;
				else
					stateInfo.nextState=NAME_S;
				stateInfo.sonsFoundName=true;
				stateInfo.descentDirection=dir;
			} else
				stateInfo.nextState=NAME_S;
	#ifdef TRUST_OLD
		} else {
			stateInfo.nextState=NAME_S;
		}
	#endif
		return ret_value;
	}
	inline void addToTree(Name & name) {
		switch (stateInfo.descentDirection) {
		case SON:
		//case UNDEFINED_DIRECTION:
			if (!currentData.last->hasSpouse(name))
				new GeneNode(name,currentData.last);
			break;
		case FATHER:
			currentData.last->addParent(new GeneNode(name,NULL));
			currentData.tree->updateRoot();
			break;
		case SPOUSE:
			if (currentData.last->name.isMarriageCompatible(name))
				currentData.last->addSpouse(name);
			else {
				DescentDirection temp=stateInfo.descentDirection;
				stateInfo.descentDirection=stateInfo.lastDescentDirection;
				addToTree(name); //TODO: check if useful, or needs more complex intervention
				stateInfo.descentDirection=temp;
				return;
			}
			break;
		case SIBLING:
			currentData.last->addSibling(new GeneNode(name,NULL));
			if (currentData.last->getParent()!=NULL)
				currentData.outputData->addName(name);
			return;
		}
		if (stateInfo.descentDirection!=UNDEFINED_DIRECTION && currentData.last!=NULL)
			currentData.outputData->addName(name);
	}
	bool getNextState(bool addCounters=true) {
		display(stateInfo.currentState);
		display(QString(" i0: %1 ").arg(currentData.i0));
		display(QString("curr: %1 ").arg(currentData.last->toString()));
		//display(QString("startGene: %1\n").arg(currentData.startGene));
		display("dir: ");display(stateInfo.descentDirection);display("\n");
	#ifdef GENEOLOGYDEBUG
		currentData.tree->outputTree();
		display("\n");
	#endif
		bool ret_val=true;
		Name name(stateInfo.text,stateInfo.startPos,stateInfo.endPos,stateInfo.male);
		switch(stateInfo.currentState) {
		case TEXT_S:
			switch (stateInfo.currentType) {
			case DC:
				currentData.initialize();
				currentData.startGene=stateInfo.startPos;
				currentData.tree=new GeneTree();
				stateInfo.nextState=NAME_S;
				break;
			case NEW_NAME:
				currentData.initialize();
				currentData.last=new GeneNode(name,NULL);
				currentData.outputData->addName(name);
				currentData.tree=new GeneTree(currentData.last);
				currentData.startGene=stateInfo.startPos;
				stateInfo.descentDirection=UNDEFINED_DIRECTION;
				stateInfo.nextState=NAME_S;
				break;
			default:
				stateInfo.nextState=TEXT_S;
			}
			break;
		case NAME_S:
			if (!stateInfo.preceededByLi) {
				switch (stateInfo.currentType) {
				case DC:
					//currentData.i0=0;
					if (currentData.last==NULL)
						currentData.last=currentData.tree->findTreeNode(currentData.lastName,stateInfo.descentDirection==SON ||stateInfo.descentDirection==FATHER);
					if (stateInfo.descentDirection==SON)
						stateInfo.nextState=SONS_S;//NAME_S;
					else
						stateInfo.nextState=NAME_S;
					stateInfo.sonsFoundName=false;
					break;
				case ENDING_PUNC:
					currentData.last=NULL;
					stateInfo.nextState=NAME_S;
					stateInfo.descentDirection=UNDEFINED_DIRECTION;
					break;
				case NEW_NAME: /*|| ((stateInfo.currentType==CORE_NAME || stateInfo.currentType==LEAF_NAME) && currentData.last==NULL)*/
					if (stateInfo.preceededByWaw) {
						if (currentData.last!=NULL && currentData.last->parent==NULL) {
							currentData.tree->deleteTree();
							currentData.outputData->clear();
							currentData.last=NULL;
							currentData.tree=NULL;
							stateInfo.nextState=TEXT_S;
							display("{Waw resulted in deletion}\n");
							stateInfo.nextState=NAME_S;
						} else {
							if (currentData.last!=NULL) {
								//new GeneNode(name,currentData.last->parent);
								addToTree(name);
								stateInfo.nextState=NAME_S;
							} else {
								ret_val=doActionNewNameAndNullLast(name); //choses also nextState
							}
						}
					} else {
						if (currentData.last==NULL) {
							ret_val=doActionNewNameAndNullLast(name); //choses also nextState
						} else {
							if (currentData.tree==NULL)
								currentData.tree=new GeneTree(currentData.last);

							if (stateInfo.descentDirection!=UNDEFINED_DIRECTION) {
								addToTree(name);
								currentData.last=currentData.tree->findTreeNode(name.getString(),true);
								currentData.i0=0;
								stateInfo.nextState=NAME_S;
							} else {
							#ifndef TRUST_OLD
								doActionNewNameAndNullLast(name); //choses also nextState
							#else
								assert(stateInfo.descentDirection==UNDEFINED_DIRECTION);
								stateInfo.nextState=NAME_S;
							#endif
							}
						}
					}
					break;
				/*case CORE_NAME:*/
				case OTHER:
					display(QString("{here threshold: %1}").arg(geneologyParameters.theta_0));
					if (currentData.i0>=geneologyParameters.theta_0) {
						ret_val=checkIfDisplay();
						stateInfo.nextState=TEXT_S;
					} else {
						if (addCounters)
							currentData.i0++;
						stateInfo.nextState=NAME_S;
					}
					break;
				case CORE_NAME:
				case LEAF_NAME:
					currentData.last=currentData.tree->findTreeNode(name.getString(),stateInfo.descentDirection==SON || stateInfo.descentDirection==FATHER);
					stateInfo.nextState=SONS_S;
					stateInfo.sonsFoundName=false;
					break;
				case PARA_PUNC:
					ret_val=doParaCheck(); //choses also nextState
					break;
				case COLON_PUNC:
					if (stateInfo.preceededBygaveBirth) {
						stateInfo.descentDirection=SON;
						stateInfo.singularDescent=false;
						stateInfo.nextState=SONS_S;
						break;
					}
				default:
					stateInfo.nextState=NAME_S;
				}
				break;
			} //in this way, code applies to both NAME_S and SONS_S bc we dont reach break
		case SONS_S:
			if (!stateInfo.preceededByLi) {
				switch (stateInfo.currentType) {
				/*case DC:
					if (stateInfo.preceededBygaveBirth)
						stateInfo.descentDirection=SON;*/
				case NEW_NAME:
					addToTree(name);
					if (stateInfo.descentDirection!=UNDEFINED_DIRECTION) {
						currentData.i0=0;
						stateInfo.sonsFoundName=true;
					}
					stateInfo.nextState=SONS_S;
					break;
				case ENDING_PUNC:
					//currentData.i0=0;
					stateInfo.nextState=NAME_S;
					if (stateInfo.descentDirection==SON /*&& stateInfo.precededBygaveBirth*/ && !stateInfo.sonsFoundName) //maybe name in next sentence
						stateInfo.descentDirection=SON;
					else {
						stateInfo.descentDirection=UNDEFINED_DIRECTION;
						currentData.last=NULL;
					}
					break;
				case PARA_PUNC:
					ret_val=doParaCheck(); //choses also nextState
					break;
				case CORE_NAME:
				case LEAF_NAME:
					currentData.last=currentData.tree->findTreeNode(name.getString(),stateInfo.descentDirection==SON || stateInfo.descentDirection==FATHER);
					stateInfo.nextState=SONS_S;
					break;
				case COLON_PUNC:
					if (stateInfo.preceededBygaveBirth) {
						stateInfo.descentDirection=SON;
						stateInfo.singularDescent=false;
						stateInfo.nextState=SONS_S;
						break;
					}
				default:
					if (addCounters)
						currentData.i0++;
					stateInfo.nextState=SONS_S;
				}
				break;
			} else {
				if (stateInfo.preceededBygaveBirth) {
					if (currentData.last!=NULL) {
						stateInfo.descentDirection=SON;
						if (!currentData.tree->findTreeNode(name.getString(),true))
							currentData.last->addSpouse(name);//make sure no duplication occurs
						stateInfo.nextState=SONS_S;
					} else {
						stateInfo.descentDirection=UNDEFINED_DIRECTION;
						stateInfo.nextState=NAME_S;
					}
				} else {
					stateInfo.descentDirection=UNDEFINED_DIRECTION;
					stateInfo.nextState=stateInfo.currentState;
				}
				break;
			}
		default:
			assert(false);
		}
		return ret_val;
	}
	inline bool result(WordType t,bool addCounters=true) {
		display(t);
		stateInfo.currentType=t;
		return getNextState(addCounters);
	}
	bool proceedInStateMachine() {//does not fill stateInfo.currType
		GeneStemmer s(stateInfo.text,stateInfo.startPos);
		stateInfo.resetCurrentWordInfo();
		long  finish;
		if (isNumber(stateInfo.text,stateInfo.startPos,finish)) {
			display("Number ");
			stateInfo.endPos=finish;
			stateInfo.nextPos=next_positon(stateInfo.text,finish,stateInfo.currentPunctuationInfo);
			display(stateInfo.text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
			return result(OTHER,false);
		}
		s();
		finish=max(s.info.finish,s.finish_pos);
		if (finish==stateInfo.startPos) {
			finish=getLastLetter_IN_currentWord(stateInfo.text,stateInfo.startPos);
		}
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(stateInfo.text,finish,stateInfo.currentPunctuationInfo);
		display(stateInfo.text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
		if (s.land) {
			stateInfo.land=true;
			if (!result(OTHER,false))
				return false;
		} else if (s.name){
			long nextpos=stateInfo.nextPos;
		#ifdef GET_WAW
			PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			if ((s.has_waw || s.has_li) && (stateInfo.currentState==NAME_S ||stateInfo.currentState==SONS_S) ) {
				if (s.has_waw) {
					display("waw ");
				} else {
					display("li ");
				}
				stateInfo.startPos=s.wawStart;
				stateInfo.endPos=s.wawEnd;
				stateInfo.nextPos=s.wawEnd+1;
				stateInfo.currentPunctuationInfo.reset();
				if (!result(OTHER,false))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				if (s.has_waw)
					stateInfo.preceededByWaw=true;
				else
					stateInfo.preceededByLi=true;
				stateInfo.land=false;
			}
			stateInfo.currentPunctuationInfo=copyPunc;
		#endif
			stateInfo.startPos=s.startStem;
			stateInfo.endPos=s.finishStem;
			stateInfo.nextPos=nextpos;
			WordType type;
			QString word=stateInfo.getWord();
			bool searchSpouseCondition=stateInfo.descentDirection==SPOUSE || stateInfo.descentDirection==UNDEFINED_DIRECTION || stateInfo.preceededBygaveBirth;
			GeneNode * node=currentData.tree->findTreeNode(word,searchSpouseCondition);
			if (node==NULL)
				type=NEW_NAME;
			else if (node->isLeaf())
				type=LEAF_NAME;
			else
				type=CORE_NAME;
			stateInfo.male=s.male;
			if (stateInfo.currentType==NEW_NAME && type==NEW_NAME && stateInfo.descentDirection==UNDEFINED_DIRECTION)//in cases similar to the start of 1 chronicles
				stateInfo.descentDirection=SON;
			if (stateInfo.land) {
				stateInfo.land=false;
				type=OTHER;
			}
			if (!(result(type)))
				return false;
		#ifdef SINGULAR_DESCENT
			if (stateInfo.singularDescent) {
				stateInfo.singularDescent=false;
				stateInfo.descentDirection=stateInfo.lastDescentDirection;
			}
		#endif
			currentData.lastName=word;
		} else if (s.descentDir!=UNDEFINED_DIRECTION) {
			stateInfo.land=false;
		#ifdef SINGULAR_DESCENT
			if (!stateInfo.singularDescent)
				stateInfo.lastDescentDirection=stateInfo.descentDirection;
			else
				stateInfo.lastDescentDirection=UNDEFINED_DIRECTION;
			stateInfo.singularDescent=!s.pluralDescent;
		#endif
			stateInfo.descentDirection=s.descentDir;
			if (s.descentDir==FATHER /*&& s.pluralDescent*/ && stateInfo.preceededBygaveBirth)
				stateInfo.descentDirection=SON;
			if (!result(DC))
				return false;
			if ((stateInfo.currentState==NEW_NAME || stateInfo.currentState==CORE_NAME || stateInfo.currentState==LEAF_NAME) && s.relativeSuffix) {
				stateInfo.descentDirection=stateInfo.lastDescentDirection;
				stateInfo.singularDescent=false;
			}
			if (!stateInfo.preceededBygaveBirth)
				stateInfo.preceededBygaveBirth=s.gaveBirth;
		} else {
			stateInfo.land=false;
			stateInfo.preceededByWaw=false;
			stateInfo.preceededByLi=false;
			if (!result(OTHER))
				return false;
		}
		stateInfo.preceededByWaw=false;
		stateInfo.preceededByLi=false;
		if (stateInfo.currentPunctuationInfo.hasEndingPunctuation() || stateInfo.currentPunctuationInfo.hasParagraphPunctuation() || stateInfo.currentPunctuationInfo.colon) {
			if (!stateInfo.currentPunctuationInfo.colon){
				stateInfo.land=false;
				stateInfo.lastDescentDirection=UNDEFINED_DIRECTION;
				stateInfo.singularDescent=false;
			}
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.startPos=stateInfo.endPos+1;
			stateInfo.endPos=stateInfo.nextPos-1;
			WordType type;
			if (stateInfo.currentPunctuationInfo.hasParagraphPunctuation())
				type=PARA_PUNC;
			else if (stateInfo.currentPunctuationInfo.colon)
				type=COLON_PUNC;
			else
				type=ENDING_PUNC;
			if (!result(type,false))
				return false;
			stateInfo.preceededBygaveBirth=false;
			return true;
		} else
			return true;
	}
	void outputAndTag() {
		long geneStart=currentData.startGene,geneEnd=stateInfo.endPos;
		prg->tag(geneStart,geneEnd-geneStart+1,Qt::darkYellow,false);
		GeneTagVisitor v(prg);
		v(currentData.tree);
	#ifdef DISPLAY_INDIVIDUAL
		displayTree(currentData.globalTree);
	#endif
		if (currentData.globalTree==NULL) {
			currentData.globalTree=currentData.tree;
			currentData.tree->outputTree();
			GeneTree * duplicate=currentData.tree->duplicateTree();
			currentData.outputData->setTree(duplicate);
		}else {
			currentData.globalTree->mergeTrees(currentData.tree);

			currentData.tree->outputTree();
			currentData.outputData->setTree(currentData.tree);
		}
		currentData.outputData->setMainInterval(currentData.startGene,stateInfo.endPos);
		outputList.append(*currentData.outputData);
		currentData.outputData->clear();
	}
	inline bool overLaps(int start1,int end1,int start2,int end2) {
		assert(start1<=end1 && start2<=end2);
		if (start1>=start2 && start1<=end2)
			return true;
		if (start2>=start1 && start2<=end1)
			return true;
		return false;
	}
	inline bool after(int start1,int end1,int start2,int end2) {
		assert(start1<=end1 && start2<=end2);
		if (start1>=end2)
			return true;
		return false;
	}
	inline bool before(int start1,int end1,int start2,int end2) {
		return after(start2,end2,start1,end1);
	}
	inline int commonNames(const SelectionList & list1, const SelectionList & list2, QSet<int> & visitedTags) {
		QSet<int> visitedTags2;
		int common=0;
		for (int i=0;i<list1.size();i++) {
			if (!visitedTags.contains(i)) {
				bool found=false;
				QString s1=Name(text,list1[i].first,list1[i].second).getString();
				for (int j=0;j<list2.size();j++) {
					if (!visitedTags2.contains(j)) {
						QString s2=Name(text,list2[j].first,list2[j].second).getString();
						if (equalNames(s1,s2) ) {
							found=true;
							visitedTags.insert(i);
							visitedTags2.insert(j);
							break;
						}
					}
				}
				if (found) {
					common++;
				}
			}
		}
		return common;
	}
	int calculateStatisticsOrAnotate() {
	#define MERGE_GLOBAL_TREE	if (globalTree==NULL) { \
									globalTree=tags[i].getTree()->duplicateTree(); \
								} else { \
									globalTree->mergeTrees(tags[i].getTree()); \
								} /*\
								globalTree->displayTree(prg);*/

		OutputDataList tags;
		QList<int> common_i,common_j;
		QVector<double> boundaryRecallList, boundaryPrecisionList, graphFoundList,graphSimilarList;
		int tagOverlapCount=0;
		GeneTree * globalTree=new GeneTree();
		QFile file(QString("%1.tags").arg(fileName).toStdString().data());
		if (file.open(QIODevice::ReadOnly))	{
			QDataStream out(&file);   // we will serialize the data into the file
			out	>> tags
				>>*globalTree;
			file.close();
		} else {
			error << "Annotation File does not exist\n";
			if (file.open(QIODevice::WriteOnly)) {
				/*for (int i=0;i<timeVector->size();i++) {
					tags.append(TimeTaggerDialog::Selection(timeVector->));
				}*/
				QDataStream out(&file);   // we will serialize the data into the file
				out << outputList;
				file.close();
				error << "Annotation File has been written from current detected expressions, Correct it before use.\n";
			}
			return -1;
		}
		qSort(tags.begin(),tags.end());
		for (int i=0;i<tags.size();i++) {
			tags[i].setText(text);
			FillTextVisitor v(text);
			v(tags[i].getTree());
		}
		FillTextVisitor v(text);
		v(globalTree);

		int i=0,j=0;
		QSet<GeneNode*> visitedNodes;
		QSet<int> visitedTags;
		while (i<tags.size() && j<outputList.size()) {
			int start1=tags[i].getMainStart(),end1=tags[i].getMainEnd(),
				start2=outputList[j].getMainStart(),end2=outputList[j].getMainEnd();
			if (overLaps(start1,end1,start2,end2) && start1!=end2) {
				int countCommon=commonNames(tags[i].getNamesList(),outputList[j].getNamesList(),visitedTags);
				int countCorrect=tags[i].getNamesList().size();
				int countDetected=outputList[j].getNamesList().size();
				if (countCommon>countDetected) {
					qDebug()<<text->mid(start1,end1-start1+1)<<"\t"<<text->mid(start2,end2-start2+1)<<"\t";
					visitedTags.clear();
					countCommon=commonNames(tags[i].getNamesList(),outputList[j].getNamesList(),visitedTags);
				}
				if (!common_j.contains(j) && !common_i.contains(i)) {//so that merged parts will not be double counted
					common_i.append(i);
					common_j.append(j);
					if (countCorrect>0)
						tagOverlapCount++;
				}
				double graphFound=0, graphSimilarContext=0;
				if (countCorrect>0) {
					boundaryRecallList.append((double)countCommon/countCorrect);
					boundaryPrecisionList.append((double)countCommon/countDetected);

					outputList[j].getTree()->compareToStandardTree(tags[i].getTree(),visitedNodes,graphFound, graphSimilarContext);
					graphFoundList.append(graphFound);
					graphSimilarList.append(graphSimilarContext);
				}
			#ifdef DETAILED_DISPLAY
				displayed_error	<</*text->mid(start1,end1-start1+1)*/i<<"\t"
								<</*text->mid(start2,end2-start2+1)*/j<<"\t"
								<<countCommon<<"/"<<countCorrect<<"\t"<<countCommon<<"/"<<countDetected<<"\n";
				displayed_error	<<"\tGraph:\t"<<graphFound<<"\t"<<graphSimilarContext<<"\n";
			#endif
				if (end1<=end2) {
					visitedNodes.clear();
					visitedTags.clear();
					//MERGE_GLOBAL_TREE
					i++;
				}
				if (end2<=end1)
					j++;
			} else if (before(start1,end1,start2,end2)) {
			#ifdef DETAILED_DISPLAY
				displayed_error	<</*text->mid(start1,end1-start1+1)*/i<<"\t"
								<<"-----\n";
			#endif
				visitedNodes.clear();
				visitedTags.clear();
				//MERGE_GLOBAL_TREE
				i++;
			} else if (after(start1,end1,start2,end2)) {
			#ifdef DETAILED_DISPLAY
				displayed_error	<<"-----\t"
								<</*text->mid(start2,end2-start2+1)*/j<<"\n";
			#endif
				j++;
			}
		}
	#ifdef DETAILED_DISPLAY
		while (i<tags.size()) {
			//int start1=tags[i].getMainStart(),end1=tags[i].getMainEnd();
			displayed_error	<</*text->mid(start1,end1-start1+1)*/i<<"\t"
							<<"-----\n";
			//MERGE_GLOBAL_TREE
			i++;
		}
		while (j<outputList.size()) {
			//int start2=outputList[j].getMainStart(),end2=outputList[j].getMainEnd();
			displayed_error	<<"-----\t"
							<</*text->mid(start2,end2-start2+1)*/j<<"\n";
			j++;
		}
	#endif
		assert(common_i.size()==common_j.size());
		int commonCount=common_i.size();
		double detectionRecall=(double)commonCount/tags.size(),
			   detectionPrecision=(double)commonCount/outputList.size(),
			   boundaryRecall=sum(boundaryRecallList)/tagOverlapCount,
			   boundaryPrecision=average(boundaryPrecisionList),
			   graphFound=sum(graphFoundList)/tagOverlapCount,
			   graphSimilar=sum(graphSimilarList)/tagOverlapCount;
		double globalGraphFound, globalGraphSimilarContext;
		currentData.globalTree->compareToStandardTree(globalTree,globalGraphFound, globalGraphSimilarContext);
		globalTree->displayTree(prg);
		currentData.globalTree->deleteTree();
		globalTree->deleteTree();
	#ifdef DETAILED_DISPLAY
		displayed_error << "-------------------------\n"
						<< "Detection:\n"
						<< "\trecall=\t"<<commonCount<<"/"<<tags.size()<<"=\t"<<detectionRecall<<"\n"
						<< "\tprecision=\t"<<commonCount<<"/"<<outputList.size()<<"=\t"<<detectionPrecision<<"\n"
						<< "Boundary:\n"
						<< "\trecall=\t\t"<<boundaryRecall<<"\n"
						<< "\tprecision=\t\t"<<boundaryPrecision<<"\n"
						<< "Local Graphs:\n"
						<< "\tfound=\t\t"<<graphFound<<"\n"
						<< "\tsimilar-context=\t"<<graphSimilar<<"\n"
						<< "Global Graph:\n"
						<< "\tfound=\t\t"<<globalGraphFound<<"\n"
						<< "\tsimilar-context=\t"<<globalGraphSimilarContext<<"\n";
	#else
		displayed_error<<tags.size()<<"\t"<<detectionRecall<<"\t"<<detectionPrecision
									<<"\t"<<boundaryRecall <<"\t"<<boundaryPrecision
									<<"\t"<<graphFound     <<"\t"<<graphSimilarContext<<"\n";
	#endif
	#if 0
		for (int i=0;i<tags.size();i++) {
			displayed_error<<text->mid(tags[i].getMainStart(),tags[i].getMainEnd()-tags[i].getMainStart())<<"\n";
		}
	#endif
		for (int i=0;i<tags.size();i++)
			tags[i].getTree()->deleteTree();
		for (int i=0;i<outputList.size();i++)
			outputList[i].getTree()->deleteTree();
		return 0;
	#undef MERGE_GLOBAL_TREE
	}
	int segmentHelper(QString * text,ATMProgressIFC *prg) {
		this->prg=prg;
		if (text==NULL)
			return -1;
		long text_size=text->size();
		currentData.initialize();
		prg->startTaggingText(*text);
		int geneCounter=1;
		stateInfo.initialize(text);
		while(stateInfo.startPos<text->length() && isDelimiter(text->at(stateInfo.startPos)))
			stateInfo.startPos++;
		prg->setCurrentAction("Parsing Biblical Text");
		for (;stateInfo.startPos<text_size;) {
			if(!(proceedInStateMachine()))
				geneCounter++;
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.startPos=stateInfo.nextPos;
			stateInfo.lastEndPos=stateInfo.endPos;
			stateInfo.previousPunctuationInfo=stateInfo.currentPunctuationInfo;
			prg->report((double)stateInfo.startPos/text_size*100+0.5);
			if (stateInfo.startPos==text_size-1)
				break;
		}
		if (stateInfo.currentState!=TEXT_S) {
			if (!checkIfDisplay())
				geneCounter++;
		}
		if (currentData.outputData!=NULL)
			delete currentData.outputData;
		prg->report(100);
		prg->finishTaggingText();
		currentData.globalTree->displayTree(prg);
		calculateStatisticsOrAnotate();
		return 0;
	}

public:
	int segment(QString input_str,ATMProgressIFC *prg)  {
		fileName=input_str;
		QFile input(input_str);
		if (!input.open(QIODevice::ReadOnly)) {
			out << "File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		text=new QString(file.readAll());
		if (text->isNull())	{
			out<<"file error:"<<input.errorString()<<"\n";
			return 1;
		}
		if (text->isEmpty()) {//ignore empty files
			out<<"empty file\n";
			return 0;
		}
		return segmentHelper(text,prg);
	}
	int segment(QString * text,ATMProgressIFC *prg)  {
		fileName="";
		return segmentHelper(text,prg);
	}
};

int genealogyHelper(QString input_str,ATMProgressIFC *prgs){
	input_str=input_str.split("\n")[0];
	GenealogySegmentor s;
	s.segment(input_str,prgs);
	return 0;
}


