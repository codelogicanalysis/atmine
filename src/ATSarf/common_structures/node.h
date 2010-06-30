#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include <QVector>
#include "common.h"
#include "Ptr.h"

class node;

class letter_node;
class result_node;

class node
{
	public:
		QList<result_node*>* result_children;
		QVector<Ptr<letter_node> >* letter_children;
	public:
#ifdef PARENT
		node * parent; //public member and can be defined by other classes such as treesearch upon traversal.
#endif
		node();
		virtual bool isLetterNode() const=0;
		virtual QString to_string(bool isAffix=false)=0;
		bool hasChildren();
		QList<result_node *>* getResultChildren();
		QVector<letter_node *>* getLetterChildren();
		letter_node* getLetterChild(QChar letter);
		void addChild(node* child);
		void removeChildren();//just remove references
		virtual ~node();
};




#endif // _NODE_H
