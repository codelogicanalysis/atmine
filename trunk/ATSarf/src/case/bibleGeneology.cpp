#include "bibleGeneology.h"
#include <QTextBrowser>
#include <assert.h>

#include "ATMProgressIFC.h"
#include "Math_functions.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <assert.h>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "stemmer.h"
#include "letters.h"
#include "diacritics.h"
#include "text_handling.h"
#include "textParsing.h"
#include "common.h"
#include "Retrieve_Template.h"


enum WordType { NEW_NAME,CORE_NAME, LEAF_NAME, DC,ENDING_PUNC,PARA_PUNC, OTHER};
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
	long operator[](int i) { return descriptions[i]; }
	int size() { return descriptions.size(); }
public:
	DescentConnectorGroup(DescentDirection dir) {
		this->dir=dir;
	}
	void addDescription(long description_id) {
		descriptions<<description_id;
	}
};
class DescentConnectors {
private:
	QList<DescentConnectorGroup> groups;
public:
	void addDescentGroup(DescentConnectorGroup group) {
		groups<<group;
	}
	DescentDirection getDirection(long description_id) {
		for (int i=0;i<groups.size();i++) {
			for (int j=0;j<groups[i].size();j++) {
				if (groups[i][j]==description_id)
					return groups[i].dir;
			}
		}
		return UNDEFINED_DIRECTION;
	}
};


GeneologyParameters geneologyParameters;
QList<int> bits_gene_NAME;
QStringList relativeSuffixes;
int bit_VERB_PERFECT;
DescentConnectors descentConnectors;

#define display_letters 30
class genealogy_stemmer: public Stemmer { //TODO: seperate ibn from possessive from 3abid and later seperate between ibn and bin
private:
	bool place;
public:
	long finish_pos;
	bool name:1;
	bool gaveBirth:1;
	DescentDirection descentDir:3;
#ifdef GET_WAW
	bool has_waw:1;
#endif
	long startStem, finishStem,wawStart,wawEnd;

	genealogy_stemmer(QString * word, int start)
#ifndef COMPARE_TO_BUCKWALTER
		:Stemmer(word,start,false)
#else
		:Stemmer(word,start,true)
#endif
	{
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start) {
		this->info.start=start;
		this->info.finish=start;
		name=false;
		descentDir=UNDEFINED_DIRECTION;
		gaveBirth=false;
		finish_pos=start;
		startStem=start;
		finishStem=start;
	#ifdef GET_WAW
		wawStart=start;
		wawEnd=start;
		has_waw=false;
	#endif
	}
#ifndef COMPARE_TO_BUCKWALTER
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
	#endif
	}

#ifdef GET_WAW
	void checkForWaw() {
		has_waw=false;
	#ifndef GET_AFFIXES_ALSO
		solution_position * p_inf=Prefix->computeFirstSolution();
		do {
			prefix_infos=&Prefix->affix_info;
	#endif
			for (int i=0;i<prefix_infos->size();i++)
				if (prefix_infos->at(i).POS=="wa/CONJ+") {
					wawStart=(i==0?Prefix->info.start:Prefix->getSplitPositions().at(i-1)-1);
					wawEnd=Prefix->getSplitPositions().at(i)-1;
					has_waw=true;
					break;
				}
	#ifndef GET_AFFIXES_ALSO
		}while (Prefix->computeNextSolution(p_inf));
		delete p_inf;
	#endif
	}
#endif

	bool analyze()	{
		if (info.finish>info.start) { //more than one letter to be tested for being a name
			descentDir=descentConnectors.getDirection(stem_info->description_id);
			if (descentDir!=UNDEFINED_DIRECTION) {
				int suffSize=Suffix->info.finish - Suffix->info.start+1;
				gaveBirth=stem_info->abstract_categories.getBit(bit_VERB_PERFECT);
				if (suffSize>0 && !gaveBirth ) {
					QString suffix=info.text->mid(Suffix->info.start,suffSize);
					for (int i=0;i<relativeSuffixes.size();i++) {
						if (equal_ignore_diacritics(suffix,relativeSuffixes[i])) {
							if (descentDir==SON)
								descentDir=FATHER;
							else if (descentDir==FATHER)
								descentDir=SON;
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
					#ifdef GET_WAW
						checkForWaw();
					#endif
					}
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
	Retrieve_Template d_spouse("description","id","name='woman' OR name LIKE '%spouse%'");
	while (d_spouse.retrieve())
		spouse.addDescription(d_spouse.get(0).toULongLong());
	DescentConnectorGroup sibling(SIBLING);
	Retrieve_Template d_sibling("description","id","name='brother' OR name='sister' OR name='sister/counterpart'");
	while (d_sibling.retrieve())
		sibling.addDescription(d_sibling.get(0).toULongLong());
	DescentConnectorGroup father(FATHER);
	Retrieve_Template d_father("description","id","name='son'");
	while (d_father.retrieve())
		father.addDescription(d_father.get(0).toULongLong());
	DescentConnectorGroup son(SON);
	Retrieve_Template d_son("description","id","name='father' OR name='mother/maternal' OR name='mother' OR name='give birth to' OR name='be born' OR name LIKE 'sons/children%'");
	while (d_son.retrieve())
		son.addDescription(d_son.get(0).toULongLong());
	descentConnectors.addDescentGroup(spouse);
	descentConnectors.addDescentGroup(sibling);
	descentConnectors.addDescentGroup(father);
	descentConnectors.addDescentGroup(son);

	//abou ... bani and kharaja minho/minhoma/minhom => non direct descent

	long abstract_VERB_PERFECT=database_info.comp_rules->getAbstractCategoryID("VERB_PERFECT");
	bit_VERB_PERFECT=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_VERB_PERFECT);

	relativeSuffixes<<QString("")+ha2<<QString("")+ha2+alef<<QString("")+ha2+meem+alef<<QString("")+ta2+ha2<<QString("")+ta2+ha2+alef<<QString("")+ta2+ha2+meem+alef;
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
		bool precededByWaw:1;
		bool precededBygaveBirth:1;
		DescentDirection descentDirection:3;
		int unused:20;
		PunctuationInfo previousPunctuationInfo,currentPunctuationInfo;
		void resetCurrentWordInfo()	{
			currentPunctuationInfo.reset();
			precededByWaw=false;
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
			precededByWaw=false;
			precededBygaveBirth=false;
		}
		StateInfo() {
			initialize(NULL);
		}
	};
	class Name {
	private:
		long start,end;
		QString * text;
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
		QString getString() {
			return text->mid(start,end-start+1);
		}
		long getStart() const{return start;}
		long getLength() const{return end-start+1;}
	};
	class GeneTree;
	class GeneNode {
	private:
		friend class GeneTree;
		void setParentHeight(int height) {
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
		int getSubTreeCount() {
			if (this==NULL)
				return 0;
			int count=1;
			for (int i=0;i<children.size();i++) {
				count+=children[i]->getSubTreeCount();
			}
			return count;
		}
		GeneNode * getNodeInSubTree(QString word) {
			if (this==NULL)
				return NULL;
			if (equal_ignore_diacritics(word,name.getString()))
				return this;
			for (int i=0;i<spouses.size();i++) {
				if (equal_ignore_diacritics(word,spouses[i].getString()))
					return this;
			}
			for (int i=0;i<children.size();i++) {
				GeneNode * found=children[i]->getNodeInSubTree(word);
				if (found!=NULL)
					return found;
			}
			return NULL;
		}
		void printTree(int indentation) {
			if (this==NULL)
				out<<"NULL";
			else {
				QString sp="";
				if (spouses.size()>0) {
					sp+="(Spouses: ";
					for (int i=0;i<spouses.size();i++) {
						sp+=" "+spouses[i].getString();
					}
					sp+=") ";
				}
				out<<QString(indentation,'>')<<"["<<height<<"]"<<name.getString()<<sp<<"\n";
				for (int i=0;i<children.size();i++) {
					children[i]->printTree(indentation+1);
				}
			}
		}

	public:
		Name name;
		GeneNode * parent;
		QList<Name> spouses;
		QList<GeneNode *> children;
		int height;

		GeneNode(Name & n, GeneNode * parent ):name(n) {
			assert(parent!=this);
			this->parent=parent;
			height=0; //since no children
			setParentHeight(1);
			if (parent!=NULL)
				parent->addChild(this);
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
		GeneNode * addSpouse(Name n) { //return itself
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
		QString toString() {
			if (this==NULL)
				return "NULL";
			else
				return name.getString();
		}
	};
	class GeneTree {
	private:
		GeneNode * root;
	public:
		GeneTree() {
			root=NULL;
		}
		GeneTree(GeneNode * root) {
			this->root=root;
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
			out<<"\n{Output}\n";
			if (this==NULL)
				out<<"NULL";
			else
				root->printTree(0);
		}
		int getTreeLevels() {
			if (this==NULL)
				return 0;
			if (root==NULL)
				return 0;
			return root->height+1;
		}
		int getTreeNodesCount() {
			if (this==NULL)
				return 0;
			return root->getSubTreeCount();
		}
		GeneNode * findTreeNode(QString word) {
			if (this==NULL)
				return NULL;
			return root->getNodeInSubTree(word);
		}
	};
	class GeneVisitor {
	private:
		void visit(const GeneNode * node) {
			if (node==NULL)
				return;
			visit(node->name,node->height);
			for (int i=0;i<node->children.size();i++)
				visit(node->children[i]);
		}

	public:
		virtual void visit(const Name & name, int height)=0;
		void operator ()(GeneTree * tree) {
			GeneNode * root=tree->getRoot();
			visit(root);
		}
	};
	class GeneTagVisitor: public GeneVisitor {
	private:
		ATMProgressIFC *prg;
	public:
		GeneTagVisitor(ATMProgressIFC *prg) {
			this->prg=prg;
		}
		void visit(const Name & name, int) {
			prg->tag(name.getStart(),name.getLength(),Qt::white,true);
		}
	};
	class StateData {
	public:
		int i0;
		GeneTree * tree;
		GeneNode * last;
		QString lastName;
		long startGene;
		StateData() {
			initialize();
		}
		void initialize() {
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
				currentData.tree->getTreeNodesCount()>=geneologyParameters.C_max;
	}
	inline bool checkIfDisplay(bool keep=false) {
		bool ret_val=true;
		if (!keep) {
			if (conditionToOutput()) {
				outputAndTag();
				ret_val= false;
			} else {
				currentData.tree->deleteTree();
			}
			currentData.last=NULL;
			currentData.tree=NULL;
			stateInfo.nextState=TEXT_S; //redundant
		}
		return ret_val;
	}
	inline bool doParaCheck() {
		int count=currentData.tree->getTreeNodesCount();
		bool keep=count<=geneologyParameters.C_max && count>1;
		bool ret_val=checkIfDisplay(keep);
		if (!keep)
			stateInfo.nextState=TEXT_S;
		else {
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
	#ifndef TRUST_OLD
		if (currentData.tree->getTreeNodesCount()==0) {
	#endif
			ret_value=doParaCheck();
			currentData.startGene=stateInfo.startPos;
			currentData.last=new GeneNode(name,NULL);
			currentData.tree=new GeneTree(currentData.last);
			currentData.i0=0;
			if (stateInfo.descentDirection!=UNDEFINED_DIRECTION)
				stateInfo.nextState=SONS_S;
			else
				stateInfo.nextState=NAME_S;
	#ifndef TRUST_OLD
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
			new GeneNode(name,currentData.last);
			break;
		case FATHER:
			currentData.last->addParent(new GeneNode(name,NULL));
			currentData.tree->updateRoot();
			break;
		case SPOUSE:
			currentData.last->addSpouse(name);
			break;
		case SIBLING:
			currentData.last->addSibling(new GeneNode(name,NULL));
			break;
		}
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
		Name name(stateInfo.text,stateInfo.startPos,stateInfo.endPos);
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
			switch (stateInfo.currentType) {
			case DC:
				//currentData.i0=0;
				stateInfo.nextState=SONS_S;//NAME_S;
				break;
			case ENDING_PUNC:
				currentData.last=NULL;
				stateInfo.nextState=NAME_S;
				stateInfo.descentDirection=UNDEFINED_DIRECTION;
				break;
			case NEW_NAME: /*|| ((stateInfo.currentType==CORE_NAME || stateInfo.currentType==LEAF_NAME) && currentData.last==NULL)*/
				if (stateInfo.precededByWaw) {
					if (currentData.last!=NULL && currentData.last->parent==NULL) {
						currentData.tree->deleteTree();
						currentData.last=NULL;
						currentData.tree=NULL;
						stateInfo.nextState=TEXT_S;
						display("{Waw resulted in deletion}\n");
						stateInfo.nextState=NAME_S;
					} else {
						if (currentData.last!=NULL) {
							new GeneNode(name,currentData.last->parent);
							stateInfo.nextState=NAME_S;
						} else {
							doActionNewNameAndNullLast(name); //choses also nextState
						}
					}
				} else {
					if (currentData.last==NULL) {
						doActionNewNameAndNullLast(name); //choses also nextState
					} else {
						if (currentData.tree==NULL)
							currentData.tree=new GeneTree(currentData.last);
						currentData.last=new GeneNode(name,currentData.last);
						currentData.i0=0;
						stateInfo.nextState=NAME_S;
					}
				}
				break;
			/*case CORE_NAME:*/
			case OTHER:
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
				currentData.last=currentData.tree->findTreeNode(name.getString());
				stateInfo.nextState=SONS_S;
				break;
			case PARA_PUNC:
				ret_val=doParaCheck(); //choses also nextState
				break;
			default:
				stateInfo.nextState=NAME_S;
			}
			break;
		case SONS_S:
			switch (stateInfo.currentType) {
			case NEW_NAME:
				addToTree(name);
				if (stateInfo.descentDirection!=UNDEFINED_DIRECTION)
					currentData.i0=0;
				stateInfo.nextState=SONS_S;
				break;
			case ENDING_PUNC:
				//currentData.i0=0;
				stateInfo.nextState=NAME_S;
				stateInfo.descentDirection=UNDEFINED_DIRECTION;
				currentData.last=NULL;
				break;
			case PARA_PUNC:
				ret_val=doParaCheck(); //choses also nextState
				break;
			case CORE_NAME:
			case LEAF_NAME:
				currentData.last=currentData.tree->findTreeNode(name.getString());
				stateInfo.nextState=SONS_S;
				break;
			default:
				if (addCounters)
					currentData.i0++;
				stateInfo.nextState=SONS_S;
			}
			break;
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
		genealogy_stemmer s(stateInfo.text,stateInfo.startPos);
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
		if (s.name){
			long nextpos=stateInfo.nextPos;
		#ifdef GET_WAW
			PunctuationInfo copyPunc=stateInfo.currentPunctuationInfo;
			if (s.has_waw && (stateInfo.currentState==NAME_S ||stateInfo.currentState==SONS_S) ) {
				display("waw ");
				stateInfo.startPos=s.wawStart;
				stateInfo.endPos=s.wawEnd;
				stateInfo.nextPos=s.wawEnd+1;
				stateInfo.currentPunctuationInfo.reset();
				if (!result(OTHER,false))
					return false;
				stateInfo.currentState=stateInfo.nextState;
				stateInfo.lastEndPos=stateInfo.endPos;
				stateInfo.precededByWaw=true;
			}
			stateInfo.currentPunctuationInfo=copyPunc;
		#endif
			stateInfo.startPos=s.startStem;
			stateInfo.endPos=s.finishStem;
			stateInfo.nextPos=nextpos;
			WordType type;
			QString word=stateInfo.getWord();
			GeneNode * node=currentData.tree->findTreeNode(word);
			if (node==NULL)
				type=NEW_NAME;
			else if (node->isLeaf())
				type=LEAF_NAME;
			else
				type=CORE_NAME;
			if (stateInfo.currentType==NEW_NAME && stateInfo.descentDirection==UNDEFINED_DIRECTION)//in cases similar to the start of 1 chronicles
				stateInfo.descentDirection=SON;
			if (!(result(type)))
				return false;
			currentData.lastName=word;
		} else if (s.descentDir!=UNDEFINED_DIRECTION) {
			stateInfo.descentDirection=s.descentDir;
			if (s.descentDir==FATHER && stateInfo.precededBygaveBirth)
				stateInfo.descentDirection=SON;
			if (!result(DC))
				return false;
			stateInfo.precededBygaveBirth=s.gaveBirth;
		} else {
			stateInfo.precededByWaw=false;
			if (!result(OTHER))
				return false;
		}
		stateInfo.precededByWaw=false;
		if (stateInfo.currentPunctuationInfo.hasEndingPunctuation() || stateInfo.currentPunctuationInfo.hasParagraphPunctuation()) {
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.startPos=stateInfo.endPos+1;
			stateInfo.endPos=stateInfo.nextPos-1;
			WordType type=(stateInfo.currentPunctuationInfo.hasParagraphPunctuation()?PARA_PUNC:ENDING_PUNC);
			stateInfo.precededBygaveBirth=false;
			return result(type,false);
		} else
			return true;
	}
	void outputAndTag() {
		long geneStart=currentData.startGene,geneEnd=stateInfo.endPos;
		prg->tag(geneStart,geneEnd-geneStart+1,Qt::darkYellow,false);
		GeneTagVisitor v(prg);
		v(currentData.tree);
		currentData.tree->outputTree();
		currentData.tree->deleteTree();
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
		prg->report(100);
		prg->finishTaggingText();
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
