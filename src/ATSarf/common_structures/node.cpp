#include "node.h"
#include "letter_node.h"

node::node()
{
	result_children=new QList<result_node*>;
	letter_children= new QVector<Ptr<letter_node> >;
}
bool node::hasChildren()
{
	return (result_children->count()!=0 || letter_children->count()!=0);
}
QList<result_node *>* node::getResultChildren()
{
	return result_children;
}
QVector<letter_node*>* node::getLetterChildren()
{
	QVector<letter_node*>* ret/*=new QVector<letter_node*>*/;
	//Ptr<letter_node> l(NULL);
	for (int i=0; i<letter_children->size();i++)
	{
		ret->append(letter_children->at(i).p);
	}
	return ret;
}
void node::addChild(node* child)
{
	if (child->isLetterNode())
	{
		letter_node* l=(letter_node*)child;
		Ptr<letter_node> p1 (l);
		QVector<Ptr<letter_node> >::iterator i =   qLowerBound(letter_children->begin(),letter_children->end(), p1);
		if (i==letter_children->end() || *i!=p1)
			letter_children->insert(i, p1);
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
		result_children->removeAt(i);
		//delete children[i];
    }
	length=letter_children->count();
	for(int i=0;i<length;i++)
	{
		letter_children->remove(i);
		//delete children[i];
	}
}

letter_node* node::getLetterChild(QChar letter)
{
	letter_node* n=new letter_node(letter);
	const Ptr<letter_node> p_n(n);
	QVector<Ptr<letter_node> >::iterator i = qBinaryFind(letter_children->begin(),letter_children->end(), p_n);
	delete n;
	if (i!=letter_children->end())
		return i->p;
	else
		return NULL;
}
node::~node()
{
	delete letter_children;
	delete result_children;
}


