#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include <QVector>
#include <QHash>
#include "common.h"
#include "../utilities/Ptr.h"

//#define BINARY_SEARCH
//#define HASH_TABLE
#define EXTENSIVE_TREE

class node;

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
		void initialize(const node& n);
	public:
#ifdef PARENT
		node * parent; //public member and can be defined by other classes such as treesearch upon traversal.
#endif
		node();
		node(const node & n);
		node& operator=(const node& n);
		virtual bool isLetterNode() const=0;
		virtual QString to_string(bool isAffix=false)=0;
		bool hasChildren();
		QList<result_node *>* getResultChildren();
		QVector<letter_node* > getLetterChildren(); //inefficient copy constructor, this function's use should be avoided
		letter_node* getLetterChild(QChar & letter);
		void addChild(node* child);
		void resetChildren();
		virtual ~node();
};




#endif // _NODE_H
