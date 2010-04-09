#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include "common.h"


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
		bool hasChildren();
		QList<node *> getChildren();
		void addChild(node* child);
		void removeChildren();//just remove references
		virtual ~node();
};


#endif // _NODE_H
