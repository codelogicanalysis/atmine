#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "letters.h"
#include <assert.h>
#include <QDebug>

void node::removeChildren()
{
	int length=result_children->count();
	for(int i=0;i<length;i++)
	{
		result_children->at(i)->removeChildren();
		delete result_children->at(i);
	}
#ifdef BINARY_SEARCH
	length=letter_children->count();
	for(int i=0;i<length;i++)
	{
		letter_children->operator [](i)->removeChildren();
		delete letter_children->at(i).p;
	}
#elif defined(HASH_TABLE)
	QList<letter_node*> lett_chil=letter_children->values();
	length=lett_chil.count();
	for(int i=0;i<length;i++)
	{
		lett_chil[i]->removeChildren();
		delete lett_chil[i];
	}
	//letter_children->clear();
#elif defined(EXTENSIVE_TREE)
	length=letter_children->count();
	for(int i=0;i<length;i++)
	{
		letter_node * child=letter_children->at(i);
		if (child !=NULL)
		{
			child->removeChildren();
			delete child;
		}
	}
#endif
}

void node::addChild(node* child)
{
	if (child->isLetterNode())
	{
		letter_node* l=(letter_node*)child;
	#ifdef BINARY_SEARCH
		Ptr<letter_node> p1 (l);
		QVector<Ptr<letter_node> >::iterator i = qLowerBound(letter_children->begin(),letter_children->end(), p1);
		if (i==letter_children->end() || *i!=p1)
			letter_children->insert(i, p1);
	#elif defined(HASH_TABLE)
		letter_children->insert(l->getLetter(),l);
	#elif defined(EXTENSIVE_TREE)
		(*letter_children)[getLetterIndex(l->getLetter())]=l;
	#endif
	}
	else
		result_children->append((result_node*)child);
#ifdef PARENT
	child->parent=this;
#endif
}
