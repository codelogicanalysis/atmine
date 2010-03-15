#ifndef TREE_H
#define TREE_H

#include <QList>
#include <QChar>
#include <QString>
#include <QQueue>
#include <QPair>
#include "sql-interface.h"

#define QUEUE
#ifndef QUEUE
#define PARENT
#endif

//#define MEMORY_EXHAUSTIVE
#define REDUCE_THRU_DIACRITICS
#ifdef REDUCE_THRU_DIACRITICS
#undef MEMORY_EXHAUSTIVE
#endif

class node
{
	private:
		QList<node *> children;
	public:
#ifdef PARENT
		node * parent; //public member and can be defined by other classes such as treesearch upon traversal.
#endif
		virtual bool isLetterNode()=0;
		virtual QString to_string(bool isAffix=false)=0;
		bool hasChildren()
		{
			return (children.count()!=0);
		}
		QList<node *> getChildren()
		{
			return children;
		}
		void addChild(node* child)
		{
			children.append(child);
#ifdef PARENT
			child->parent=this;
#endif
		}
		void removeChildren()//just remove references
		{
			int length=children.count();
			for(int i=0;i<length;i++)
			{
				children.removeAt(i);
				//delete children[i];
			}
		}
		virtual ~node()	{}
		/*{
			int length=children.length();
			for(int i=0;i<length;i++)
				children.removeAt(i);
		}*/
};
class letter_node:public node
{
	private:
		QChar letter;
	public:
		letter_node(QChar letter)
		{
			setLetter(letter);
		}
		bool isLetterNode()
		{
			return true;
		}
		QChar getLetter()
		{
			return letter;
		}
		void setLetter(QChar letter)
		{
			this->letter=letter;
		}
		QString to_string(bool =true)
		{
			return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
		}
		~letter_node(){	}
};
class result_node:public node
{
	private:
		long previous_category_id;
		long affix_id;
		long resulting_category_id;
	public:

#ifdef REDUCE_THRU_DIACRITICS
		QList<QString> raw_datas;

		void add_raw_data(QString raw_data)
		{
			if (!raw_datas.contains(raw_data))
				this->raw_datas.append(raw_data);
		}
#endif

#ifdef MEMORY_EXHAUSTIVE
		//TODO: change Pair to a public class outside result_node
		class Additional
		{
			QString raw_data;
			QString description;
			QString
		} ;
		typedef QList<StringTriple> StringTriples;
		StringPairs rawdata_description; //made public to reduce copying of strings

		void addPair(QString raw_data,QString description)
		{
			if (!rawdata_description.contains(StringPair(raw_data,description)))
				rawdata_description.append(StringPair(raw_data,description));
		}

		result_node(long affix_id,long previous_category_id,long resulting_category_id, QString raw_data, QString description)
		{
			rawdata_description.clear();
			addPair(raw_data,description);
#elif defined (REDUCE_THRU_DIACRITICS)
		result_node(long affix_id,long previous_category_id,long resulting_category_id,QString raw_data)
		{
			this->raw_datas.clear();
			add_raw_data(raw_data);
#else
		result_node(long affix_id,long previous_category_id,long resulting_category_id)
		{
#endif
			set_previous_category_id(previous_category_id);
			set_resulting_category_id(resulting_category_id);
			set_affix_id(affix_id);
		}
		bool isLetterNode()
		{
			return false;
		}
		long get_previous_category_id()
		{
			return previous_category_id;
		}
		void set_previous_category_id(long id)
		{
			previous_category_id=id;
		}
		long get_affix_id()
		{
			return affix_id;
		}
		void set_affix_id(long id)
		{
			affix_id=id;
		}
		long get_resulting_category_id()
		{
			return resulting_category_id;
		}
		void set_resulting_category_id(long id)
		{
			resulting_category_id=id;
		}
		QString to_string(bool isAffix=true)
		{
			return QString("-").append(QString("%1").arg((isAffix?getColumn("category","name",previous_category_id):QString("%1").arg(previous_category_id)))).append(">[").append(QString("%1").arg((isAffix?getColumn("category","name",resulting_category_id):QString("%1").arg(resulting_category_id)))).append("]");
		}
		~result_node(){	}
};

class tree
{
protected:
	node* base;
	int letter_nodes, result_nodes;
	bool isAffix;
	item_types type;
	void delete_helper(node * current)
	{
		QList<node *> children=current->getChildren();
		node* child;
		foreach (child,children)
		{
			delete_helper(child);
			delete child;
		}
	}
	void print_tree_helper(node * current_node, int level)
	{
			out<<QString().fill(' ',level*7)<<current_node->to_string(isAffix)<<"\n";
			QList<node*> list=current_node->getChildren();
			for(int i=0;i<list.count();i++)
					print_tree_helper(list.at(i),level+1);
	}
	int build_helper(item_types type, long cat_id1, int size, node * current)
	{
		if (size<=0)
			return 0;
		long cat_id2,cat_r_id;
		Search_Compatibility s2((type==PREFIX?AA:CC),cat_id1);
		while (s2.retrieve(cat_id2,cat_r_id))
		{
			Search_by_category s3(cat_id2);
#ifdef MEMORY_EXHAUSTIVE
			all_item_info inf;
			while(s3.retrieve(inf))
			{
				QString name= getColumn(interpret_type(type),"name",inf.item_id);
				node * next=addElement(name,inf.item_id,cat_id2,cat_r_id,inf.raw_data,inf.description,current);
#elif defined(REDUCE_THRU_DIACRITICS)
			all_item_info inf;
			while(s3.retrieve(inf))
			{
				QString name= getColumn(interpret_type(type),"name",inf.item_id);
				node * next=addElement(name,inf.item_id,cat_id2,cat_r_id,inf.raw_data,current);
#else
			long long affix_id;
			while(s3.retrieve(affix_id))
			{
				QString name= getColumn(interpret_type(type),"name",affix_id);
				node * next=addElement(name,affix_id,cat_id2,cat_r_id,current);

#endif
				build_helper(type,cat_r_id,size-name.length(),next);
			}
		}
		return 0;
	}
#ifdef MEMORY_EXHAUSTIVE
	node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,QString raw_data,QString description,node * current)
#elif defined (REDUCE_THRU_DIACRITICS)
	node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,QString raw_data,node * current)
#else
	node* addElement(QString letters, long affix_id,long category_id, long resulting_category_id,node * current)
#endif
	{
		//pre-condition: assumes category_id is added to the right place and results in the appropraite resulting_category
		if (current->isLetterNode() && current!=base)
		{
			error << "Unexpected Error: provided node was a letter node and not a result one\n";
			return NULL;
		}
		QChar current_letter;
		QList<node *> current_children;
		int i,j;
		if (letters.count()==0)
		{
			current_letter='\0';
			if (current==base)
				goto result;
		}
		else
			current_letter=letters[0];
		current_children=current->getChildren();
		i=0;
		do
		{
			int num_children=current_children.count();
			for (j=0;j<num_children;j++)
			{
				if (current_children[j]->isLetterNode())
					if (((letter_node*)current_children[j])->getLetter()==current_letter)
					{
						current=current_children[j];
						current_children=current->getChildren();
						i++;
						current_letter=letters[i];
						break;
					}
			}
			if (j>=num_children)//old num_children is required in case previous if statement was successful
				break;
		}while(i<letters.count());
		if (letters.count()==0 && i==0)
		{
			//add null letter
			letter_node* new_node=new letter_node('\0');
			current->addChild(new_node);
			current=new_node;
			letter_nodes++;
		}
		for (;i<letters.count();i++)
		{
			//add necessary letters
			letter_node* new_node=new letter_node(letters[i]);
			current->addChild(new_node);
			current=new_node;
			letter_nodes++;
		}
result:	node * old_result;
		foreach (old_result,current->getChildren()) //check if this result node is already present
		{
			if (((result_node*)old_result)->get_previous_category_id()==category_id && ((result_node*)old_result)->get_resulting_category_id()==resulting_category_id && ((result_node*)old_result)->get_affix_id()==affix_id)
			{
#ifdef MEMORY_EXHAUSTIVE
				((result_node*)old_result)->addPair(raw_data,description);
#elif defined(REDUCE_THRU_DIACRITICS)
				((result_node*)old_result)->add_raw_data(raw_data);
#endif
				return old_result;
			}
		}
#ifdef MEMORY_EXHAUSTIVE
		result_node * result=new result_node(affix_id,category_id,resulting_category_id,raw_data,description);
#elif defined(REDUCE_THRU_DIACRITICS)
		result_node * result=new result_node(affix_id,category_id,resulting_category_id,raw_data);
#else
		result_node * result=new result_node(affix_id,category_id,resulting_category_id);
#endif
		current->addChild(result);
		current=result;
		result_nodes++;
		return current;
		//post-condition: returns node of resulting category reached after addition
	}
public:
	tree()
	{
		base= new letter_node('\0');
		letter_nodes=1;
		result_nodes=0;
		isAffix=false;
	}
	tree(item_types type)
	{
		base= new letter_node('\0');
		letter_nodes=1;
		result_nodes=0;
		isAffix=true;
		build_affix_tree(type);
	}
	bool getAffixType(item_types &type)
		{
			if (isAffix)
			{
				type=this->type;
				return true;
			}
			else
				return false;
		}
	node* getFirstNode()
	{
		return base;
	}
#if !defined(MEMORY_EXHAUSTIVE) && !defined(REDUCE_THRU_DIACRITICS)
	void sample()
	{
		reset();
		letter_node * A0=new letter_node('A');
		letter_node * B0=new letter_node('B');
		letter_node * C0=new letter_node('C');
		letter_node * D0=new letter_node('D');
		letter_node * B1=new letter_node('B');
		letter_node * C1=new letter_node('C');
		letter_node * D1=new letter_node('D');
		letter_node * C3=new letter_node('C');
		letter_node * D3=new letter_node('D');
		letter_node * F1=new letter_node('F');
		letter_node * E1=new letter_node('E');
		letter_node * G1=new letter_node('G');
		letter_node * G2=new letter_node('G');
		letter_node * F2=new letter_node('F');
		result_node * rc1= new result_node(-1,0,1);
		result_node * rc2= new result_node(-1,2,2);
		result_node * rc3= new result_node(-1,0,3);
		result_node * rc5= new result_node(-1,3,5);
		result_node * rc6= new result_node(-1,4,6);
		result_node * rc7= new result_node(-1,9,7);
		result_node * rc8= new result_node(-1,1,8);
		result_node * rc9= new result_node(-1,1,9);
		result_node * rc9_= new result_node(-1,3,9);
		base->addChild(A0);
		A0->addChild(rc1);
		A0->addChild(B0);
		rc1->addChild(B1);
		B1->addChild(rc2);
		rc2->addChild(C0);
		C0->addChild(D0);
		D0->addChild(rc3);
		B0->addChild(rc5);
		rc5->addChild(C1);
		C1->addChild(D1);
		D1->addChild(rc6);
		B0->addChild(C3);
		C3->addChild(rc7);
		rc7->addChild(D3);
		rc7->addChild(F1);
		D3->addChild(E1);
		E1->addChild(rc8);
		F1->addChild(G1);
		G1->addChild(rc9);
		C3->addChild(F2);
		F2->addChild(G2);
		G2->addChild(rc9_);
		letter_nodes=15;
		result_nodes=9;
		isAffix=false;
	}
#endif
	int build_affix_tree(item_types type)
	{
		reset();
		isAffix=true;
		this->type=type;
		QSqlQuery query(db);
		QString stmt=QString("SELECT id, name FROM %1").arg(interpret_type(type));
		QString name;
		unsigned long affix_id1;
		bool ok;
		if (!execute_query(stmt,query))
			return -1;;
		while (query.next())
		{
			name=query.value(1).toString();
			affix_id1=query.value(0).toLL();
			Search_by_item s1(type,affix_id1);
#ifdef MEMORY_EXHAUSTIVE
			minimal_item_info inf;
			while(s1.retrieve(inf))
			{
				node * next=addElement(name,affix_id1,inf.category_id,inf.category_id,inf.raw_data,inf.description,base);
				build_helper(type,inf.category_id,6-name.length(),next);
			}
#elif defined(REDUCE_THRU_DIACRITICS)
			minimal_item_info inf;
			while(s1.retrieve(inf))
			{
				node * next=addElement(name,affix_id1,inf.category_id,inf.category_id,inf.raw_data,base);
				build_helper(type,inf.category_id,6-name.length(),next);
			}
#else
			long cat_id;
			while(s1.retrieve(cat_id))
			{
				node * next=addElement(name,affix_id1,cat_id,cat_id,base);
				build_helper(type,cat_id,6-name.length(),next);
			}

#endif
		}
		return 0;
	}
	void reset()
	{
		delete_helper(base);
		base->removeChildren();
		letter_nodes=1;
		result_nodes=0;
		isAffix=false;
	}
	void print_tree()
	{
		if (isAffix)
			out	<<QString().fill('-',40)<<"\n"
				<<"\t"<<interpret_type(type)<<" Tree\n"
				<<QString().fill('-',40)<<"\n";
		print_tree_helper(base,0);
		out	<<QString().fill('-',40)<<"\n"
			<<"letter nodes count= "<<letter_nodes<<"\n"
			<<"result nodes count= "<<result_nodes<<"\n"
			<<QString().fill('-',40)<<"\n";
	}
	virtual ~tree()
	{
		reset();
		delete base;
	}
};

#endif // TREE_H
