#ifndef TREE_H
#define TREE_H

#include <QList>

template<class NodeType, class EdgeType>
class Node {
private:
	QList<Node<NodeType, EdgeType> * > children;
	QList<EdgeType > edges;
	NodeType value;
public:
	Node(const NodeType & val): value(val) {}
	const NodeType & getValue() const { return value;}
	Node<NodeType, EdgeType> * getChild(int i) { return children[i];}
	const EdgeType & getEdge(int i) const {return edges[i];}
	int size() const { return children.size();}
	void addNode(const EdgeType & edge,Node<NodeType, EdgeType> * node) { children.append(node); edges.append(edge);}
	bool isLeaf() const {return size()==0;}
};


template <class NodeType, class EdgeType>
class Tree {
private:
	Node<NodeType, EdgeType> * root;
private:
	void deleteHelper(Node<NodeType, EdgeType> * subroot) {
		for (int i=0;i<subroot->size();i++) {
			delete subroot->getChild(i);
		}
		delete subroot;
	}

public:
	Tree() { root=NULL;}
	Tree(Node<NodeType, EdgeType> * root) {this->root=root;}
	void setRoot(Node<NodeType, EdgeType> * root) {this->root=root;}
	Node<NodeType, EdgeType> * getRoot() const {return root;}
	~Tree() {deleteHelper(root);}
};

#endif // TREE_H
