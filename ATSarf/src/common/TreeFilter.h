#ifndef TREEFILTER_H
#define TREEFILTER_H

#include "TreeVisitor.h"


template <class NodeType, class EdgeType>
class TreeFilter: public TreeVisitor<NodeType, EdgeType> {
private:
	QList<Node<NodeType, EdgeType> *> toDelete;
protected:
	virtual bool shouldBeRemoved(Node<NodeType, EdgeType> *node)=0;
	virtual void initialize() {}
	virtual bool visitNode(Node<NodeType, EdgeType> * node) {
		if (node==NULL)
			return false;
		if (shouldBeRemoved(node)) {
			Node<NodeType, EdgeType> * parent=NULL;
			Node<NodeType, EdgeType> * nodeTemp=node;
			do {
				parent=nodeTemp->getParent();
				parent->removeChild(nodeTemp, true);
				toDelete.append(nodeTemp);
				nodeTemp->clearChildren();
				nodeTemp=parent;
			} while (parent!=NULL && parent->size(true)==1);
			return false; //dont continue deeper in this node
		}
		return true;
	}
	virtual bool visitEdge(Node<NodeType, EdgeType> *, int ) { return true;}
	virtual void finish() {
		for (int i=0;i<toDelete.size();i++) {
			Node<NodeType, EdgeType> * node=toDelete[i], * parent=node->getParent();
			if (parent!=NULL)
				parent->removeNullChilds();
			delete node;
		}
		toDelete.clear();
	};
};

#endif // TREEFILTER_H
