#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include <QVector>
#include <QHash>
#include "common.h"
#include "Ptr.h"

//#define BINARY_SEARCH
//#define HASH_TABLE
#define EXTENSIVE_TREE

inline int getLetterIndex(const QChar & letter)
{
	int unicode=letter.unicode();
	if (unicode==0)
		return 36;
	static int letter_map[42]={0 /*0x621*/, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25/*0x63A*/,
										-1,-1,-1,-1,-1,-1,26 /*0x641*/,27,28,29,30,31,32,33,34,35};

	int diff=unicode-1569/*hamza.unicode()*/;
	if (!(diff>0 && diff <42))
		return -1;
	return letter_map[diff];
}

class letter_node;
class result_node;

class node
{
	private:
		QList<result_node*>* result_children;
#ifdef BINARY_SEARCH
		QVector<Ptr<letter_node> >* letter_children;
#elif defined(HASH_TABLE)
		QHash<QChar,letter_node*>* letter_children;
#elif defined(EXTENSIVE_TREE)
		QVector<letter_node*>* letter_children;
#endif
		void removeChildren();
		void initialize(const node& n)
		{
		#ifdef PARENT
			parent=n.parent;
		#endif
		#ifdef BINARY_SEARCH
			letter_children= new QVector<Ptr<letter_node> >(*n.letter_children);
		#elif defined(HASH_TABLE)
			letter_children=new QHash<QChar,letter_node *>(*n.letter_children);
		#elif defined(EXTENSIVE_TREE)
			letter_children=new QVector<letter_node *>(*n.letter_children);
		#endif
			result_children=new QList<result_node*>(*n.result_children);
		}
	public:
#ifdef PARENT
		node * parent; //public member and can be defined by other classes such as treesearch upon traversal.
		node * getPrevious() {
			return parent;
		}
#endif
		node():parent(NULL)
		{
			result_children=new QList<result_node*>;
		#ifdef BINARY_SEARCH
			letter_children= new QVector<Ptr<letter_node> >;
		#elif defined(HASH_TABLE)
			letter_children=new QHash<QChar,letter_node *>();
		#elif defined(EXTENSIVE_TREE)
			letter_children=new QVector<letter_node *>(37);
		#if 0 //not needed it seems bc by default initialized to NULL
			for (int i=0;i<letter_children->count();i++)
				letter_children->operator [](i)=NULL;
		#endif
		#endif
		}
		node(const node & n)
		{
			initialize(n);
		}
		node& operator=(const node& n)
		{
			initialize(n);
			return *this;
		}
		virtual bool isLetterNode() const=0;
		virtual QString to_string(bool isAffix=false)=0;
		bool hasChildren()
		{
			return (result_children->count()!=0 || letter_children->count()!=0);
		}
		QList<result_node *>* getResultChildren()
		{
			return result_children;
		}
		QVector<letter_node* > getLetterChildren() //inefficient copy constructor, this function's use should be avoided
		{
			#ifdef BINARY_SEARCH
				QVector<letter_node*> ret;
				for (int i=0; i<letter_children->size();i++)
					ret.append(letter_children->at(i).p);
				return ret;
			#elif defined(HASH_TABLE)
				return letter_children->values().toVector();
			#elif defined(EXTENSIVE_TREE)
				return *letter_children; //must be changed later to filter empty nodes
			#endif
		}
		letter_node* getLetterChild(QChar & letter)
		{
		#ifdef BINARY_SEARCH
			letter_node* n=new letter_node(letter);
			const Ptr<letter_node> p_n(n);
			QVector<Ptr<letter_node> >::iterator i = qBinaryFind(letter_children->begin(),letter_children->end(), p_n);
			delete n;
			if (i!=letter_children->end())
				return i->p;
			else
				return NULL;
		#elif defined(HASH_TABLE)
			return letter_children->value(letter,NULL);
		#elif defined(EXTENSIVE_TREE)
			int i=getLetterIndex(letter);
			if (i>=0)
				return letter_children->at(i);
			else
				return NULL;
		#endif
		}
		void addChild(node* child);
		void resetChildren()
		{
			removeChildren();
			result_children->clear();
		#ifndef EXTENSIVE_TREE
			letter_children->clear();
		#endif
		}
		virtual ~node()
		{
			//removeChildren();
			delete letter_children;
			delete result_children;
		}
};




#endif // _NODE_H
