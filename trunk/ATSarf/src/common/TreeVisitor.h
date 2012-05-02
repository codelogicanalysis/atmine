#ifndef TREEVISITOR_H
#define TREEVISITOR_H

#include "Tree.h"

template<class NodeType, class EdgeType>
class TreeVisitor {
protected:
	virtual void initialize()=0;
	virtual bool visitNode(Node<NodeType,EdgeType> * node)=0;
	virtual bool visitEdge(Node<NodeType,EdgeType> * node, int edgeIndex)=0;
	virtual void finish()=0;
private:
	void dfsHelper(Node<NodeType,EdgeType> * node) {
		if (!visitNode(node))
			return;

		for (int i=0;i<node->size();i++) {
			Node<NodeType,EdgeType> * child=node->getChild(i);
			if (!visitEdge(node,i))
				return;
			dfsHelper(child);
		}
	}
public:
	void DFS(Tree<NodeType,EdgeType> & tree) {
		initialize();
		dfsHelper(tree.getRoot());
		finish();
	}
};


#endif // TREEVISITOR_H
