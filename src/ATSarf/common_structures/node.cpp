#include "node.h"
#include "letter_node.h"
#include "result_node.h"
#include "../utilities/letters.h"
#include <assert.h>
#include <QDebug>

inline int getLetterIndex(const QChar & letter)
{
	static int letter_map[42]={0 /*0x621*/, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25/*0x63A*/,
										-1,-1,-1,-1,-1,-1,26 /*0x641*/,27,28,29,30,31,32,33,34,35};

	int diff=letter.unicode()-1569/*hamza.unicode()*/;
	if (!(diff>0 && diff <42))
		return -1;
	return letter_map[diff];
}

node::node()
{
	result_children=new QList<result_node*>;
#ifdef BINARY_SEARCH
	letter_children= new QVector<Ptr<letter_node> >;
#elif defined(HASH_TABLE)
	letter_children=new QHash<QChar,letter_node *>();
#elif defined(EXTENSIVE_TREE)
	letter_children=new QVector<letter_node *>(36);
#if 0 //not needed it seems bc by default initialized to NULL
	for (int i=0;i<letter_children->count();i++)
		letter_children->operator [](i)=NULL;
#endif
#endif
}

void node::initialize(const node & n)
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

node::node(const node & n)
{
	initialize(n);
}
node& node::operator=(const node& n)
{
	initialize(n);
	return *this;
}
bool node::hasChildren()
{
	return (result_children->count()!=0 || letter_children->count()!=0);
}

QList<result_node *>* node::getResultChildren()
{
	return result_children;
}

QVector<letter_node* > node::getLetterChildren()
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
		letter_children->operator [](getLetterIndex(l->getLetter()))=l;
	#endif
	}
	else
		result_children->append((result_node*)child);
#ifdef PARENT
    child->parent=this;
#endif
}
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

void node::resetChildren()
{
	removeChildren();
	result_children->clear();
#ifndef EXTENSIVE_TREE
	letter_children->clear();
#endif
}

letter_node* node::getLetterChild(QChar& letter)
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
node::~node()
{
	//removeChildren();
	delete letter_children;
	delete result_children;
}


