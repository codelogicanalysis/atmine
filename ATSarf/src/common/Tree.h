#ifndef TREE_H
#define TREE_H

#include <QList>
#include <QDataStream>

template<class NodeType, class EdgeType>
class Node {
private:
	Node<NodeType, EdgeType> * parent;
	QList<Node<NodeType, EdgeType> * > children;
	QList<EdgeType > edges;
	NodeType value;
protected:

public:
	Node(){parent=NULL;}
	Node(const NodeType & val): value(val) {parent=NULL;}
	const NodeType & getValue() const { return value;}
	void setValue(const NodeType & val) { value=val;}
	Node<NodeType, EdgeType> * getChild(int i) const { return children[i];}
	Node<NodeType, EdgeType> * getParent() const {return parent;}
	const EdgeType & getEdge(int i) const {return edges[i];}
	int size(bool verifyIfNull=false) const {
		if (!verifyIfNull)
			return children.size();
		int num=0;
		for (int i=0;i<children.size();i++)
			if (children[i]!=NULL)
				num++;
		return num;
	}
	void addNode(const EdgeType & edge,Node<NodeType, EdgeType> * node) {
		children.append(node); edges.append(edge);
		if (node->parent!=NULL)
			node->parent->removeChild(node);
		node->parent=this;
	}
	void removeChild(int i, bool setNull=false)  {
		if (setNull)
			children[i]=NULL;
		else {
			edges.removeAt(i);children.removeAt(i);
		}
	}
	int removeNullChilds() {
		int num=0;
		for (int i=0;i<size();i++) {
			if (children[i]==NULL) {
				removeChild(i,false);
				num++;
			}
		}
		return num;
	}
	bool removeChild(Node<NodeType, EdgeType> * child, bool setNull=false)  {
		for (int i=0;i<size();i++) {
			if (children[i]==child) {
				removeChild(i, setNull);
				return true;
			}
		}
		return false;
	}
	void clearChildren() { edges.clear();children.clear();}
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
	void setRoot(Node<NodeType, EdgeType> * root) {this->root=root; /*root->parent=NULL;*/} //allow for a tree to be a subtree also
	Node<NodeType, EdgeType> * getRoot() const {return root;}
	~Tree() {deleteHelper(root);}
};

template <class NodeType, class EdgeType>
inline QDataStream &operator<<(QDataStream &out, const Node<NodeType, EdgeType> &t) {
	out<<t.getValue();
	int size=t.size();
	out<<size;
	for (int i=0;i<size;i++) {
		out<<t.getEdge(i);
		out<<*(t.getChild(i));
	}
	return out;
}

template <class NodeType, class EdgeType>
inline QDataStream &operator>>(QDataStream &in, Node<NodeType, EdgeType> &t) {
	NodeType val;
	in>>val;
	t.setValue(val);
	int size;
	in >>size;
	for (int i=0;i<size;i++) {
		EdgeType e;
		in >>e;
		Node<NodeType, EdgeType> * child=new Node<NodeType, EdgeType>();
		in>>*(child);
		t.addNode(e,child);
	}
	return in;
}

template <class NodeType, class EdgeType>
inline QDataStream &operator<<(QDataStream &out, const Tree<NodeType, EdgeType> &t) {
	out<<*(t.getRoot());
	return out;
}

template <class NodeType, class EdgeType>
inline QDataStream &operator>>(QDataStream &in, Tree<NodeType, EdgeType> &t) {
	Node<NodeType, EdgeType> * root=new Node<NodeType, EdgeType>();
	in>>*root;
	t.setRoot(root);
	return in;
}


#endif // TREE_H
