#ifndef TREE_H
#define TREE_H

#include <QList>
#include <QChar>
#include <QString>

class node
{
	private:
		QList<node *> children;
	public:
		virtual bool isLetterNode()=0;
		virtual QString to_string()=0;
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
		QString to_string()
		{
			return QString("--->(").append((letter=='\0'?'$':letter)).append(")");
		}
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
		QString to_string()
		{
			return QString("-").append(QString("%1").arg(previous_category_id)).append(">[").append(QString("%1").arg(resulting_category_id)).append("]");
		}
};

class tree
{
private:
	node* base;
	int num_nodes;

	void print_tree_helper(node * current_node, int level)
	{
		out<<QString().fill(' ',level*7)<<current_node->to_string()<<"\n";
		QList<node*> list=current_node->getChildren();
		for(int i=0;i<list.count();i++)
			print_tree_helper(list.at(i),level+1);

	}
public:
	tree()
	{
		base= new letter_node('\0');
	}
	void sample()
	{
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

	}
	node* addElement(QString letters,long category_id, long resulting_category_id)
	{
		return addElement(letters,category_id, resulting_category_id,base);
	}
	node* addElement(QString letters,long category_id, long resulting_category_id,node * current)
	{
		//pre-condition: assumes category_id is added to the right place and results in the appropraite resulting_category

		return current;
		//post-condition: returns node of resulting category reached after addition
	}
	void print_tree()
	{
		print_tree_helper(base,0);
	}
        void traverse_text(QString original_word, int starting_position,bool * on_match(...))
	{
            //traverse according to 'original_word' starting from 'starting_position' and when a match is reached calls 'on_match(original_word,new_position,...)
            //if on_match() returns true continue, else stop
        }
};

#endif // TREE_H
