#include "node.h"

bool node::hasChildren()
{
    return (children.count()!=0);
}
QList<node *>* node::getChildren()
{
	return &children;
}
void node::addChild(node* child)
{
    children.append(child);
#ifdef PARENT
    child->parent=this;
#endif
}
void node::removeChildren()//just remove references
{
    int length=children.count();
    for(int i=0;i<length;i++)
    {
            children.removeAt(i);
            //delete children[i];
    }
}
node::~node() {}


