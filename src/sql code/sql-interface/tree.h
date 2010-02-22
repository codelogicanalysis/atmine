#ifndef TREE_H
#define TREE_H

#include <QList>
#include <QChar>
#include <QString>
#include "sql-interface.h"

class node
{
	private:
		QList<node *> children;
	public:
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
		}
		void removeChildren()//just remove references
		{
			int length=children.length();
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
		QString to_string(bool isAffix=true)
		{
			return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
		}
		~letter_node(){	}
};
class result_node:public node
{
	private:
		long previous_category_id;
		long resulting_category_id;
	public:
		result_node(long previous_category_id,long resulting_category_id)
		{
			set_previous_category_id(previous_category_id);
			set_resulting_category_id(resulting_category_id);
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
private:
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
		long long affix_id;
		long cat_id2,cat_r_id;
		Search_Compatibility s2(AA,cat_id1);
		while (s2.retrieve(cat_id2,cat_r_id))
		{
			Search_by_category s3(cat_id2);
			while(s3.retrieve(affix_id))
			{
				QString name= getColumn(interpret_type(type),"name",affix_id);
				node * next=addElement(name,cat_id2,cat_r_id,current);
				build_helper(type,cat_r_id,size-name.length(),next);
			}
		}
		return 0;
	}
	node* addElement(QString letters, long category_id, long resulting_category_id)
	{
		return addElement(letters,category_id, resulting_category_id,base);
	}
	node* addElement(QString letters, long category_id, long resulting_category_id,node * current)
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
result:	result_node * result=new result_node(category_id,resulting_category_id);
		current->addChild(result);
		current=result;
		result_nodes++;
		return current;
		//post-condition: returns node of resulting category reached after addition
	}

	virtual bool on_match_helper(int match_pos,QList<long> cats) //useless here nedded just for purpose of TreeSearch
	{
		out<<match_pos<<"\n";
		return true;
	}
public:
	tree()
	{
		base= new letter_node('\0');
		letter_nodes=1;
		result_nodes=0;
		isAffix=false;
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
		result_node * rc1= new result_node(0,1);
		result_node * rc2= new result_node(2,2);
		result_node * rc3= new result_node(0,3);
		result_node * rc5= new result_node(3,5);
		result_node * rc6= new result_node(4,6);
		result_node * rc7= new result_node(9,7);
		result_node * rc8= new result_node(1,8);
		result_node * rc9= new result_node(1,9);
		result_node * rc9_= new result_node(3,9);
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
			long cat_id;
			Search_by_item s1(type,affix_id1);
			while(s1.retrieve(cat_id))
			{
				node * next=addElement(name,cat_id,cat_id,base);
				build_helper(type,affix_id1,6-name.length(),next);
			}
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
	void traverse_text(QString original_word, int starting_position)
	{
			//traverse according to 'original_word' starting from 'starting_position' and when a match is reached calls 'on_match_helper'
			//if on_match_helper() returns true continue, else stop
		}
	virtual ~tree()
	{
		reset();
		delete base;
	}
};

#endif // TREE_H
