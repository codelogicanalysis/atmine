#include "node.h"
#include "letter_node.h"
#include "result_node.h"

node::node()
{
	result_children=new QList<result_node*>;
#ifdef BINARY_SEARCH
	letter_children= new QVector<Ptr<letter_node> >;
#elif defined(HASH_TABLE)
	letter_children=new QHash<QChar,letter_node *>();
#endif
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
	#endif
	}
	else
		result_children->append((result_node*)child);
#ifdef PARENT
    child->parent=this;
#endif
}
void node::removeChildren()//just remove references
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
		letter_children->at(i).p.removeChildren();
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
#endif
}

void node::resetChildren()
{
	removeChildren();
	result_children->clear();
	letter_children->clear();
}

letter_node* node::getLetterChild(QChar letter)
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
#endif
}
node::~node()
{
	//removeChildren();
	delete letter_children;
	delete result_children;
}


