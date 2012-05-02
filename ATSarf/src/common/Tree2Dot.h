#ifndef TREE2DOT_H
#define TREE2DOT_H


#include <QTextStream>
#include <QFile>
#include <assert.h>
#include "logger.h"
#include "TreeVisitor.h"


template <class NodeType, class EdgeType>
class Tree2Dot: public TreeVisitor<NodeType, EdgeType> {
private:
	QString fileName;
	QFile * file;
	QTextStream * dot_out;
	#define d_out (*dot_out)
private:
	QString getNodeId(Node<NodeType, EdgeType> *node) const {
		return QString("s%1").arg((long)node);
	}
protected:
	virtual void initialize() {
		assert (file==NULL);
		assert(dot_out==NULL);
		file=new QFile(fileName);
		file->remove();
		if (!file->open(QIODevice::ReadWrite)) {
			out<<"Error openning '"<<fileName<<"'' file\n";
			finish();
			return;
		}

		dot_out=new QTextStream(file);
		d_out.setCodec("utf-8");
		d_out<<"digraph ggraph {\n";
	}
	virtual bool visitEdge(Node<NodeType, EdgeType> *node1, int edgeIndex) {
		Node<NodeType, EdgeType> *node2=node1->getChild(edgeIndex);
		QString s1=getNodeId(node1),s2=getNodeId(node2);
		QString e=node1->getEdge(edgeIndex);
		QString additionalEdge=getExtraEdgeLayout(node1,edgeIndex);
		if (!additionalEdge.isEmpty())
			additionalEdge=QString(", ")+additionalEdge;
		d_out<<s1<<"->"<<s2<<"[label=\""<<e<<"\""<<additionalEdge<<"];\n";
		return true;
	}
	virtual bool visitNode(Node<NodeType, EdgeType> * node) {
		QString v=node->getValue();
		QString additionalNode=getExtraNodeLayout(node);
		if (!additionalNode.isEmpty())
			additionalNode=QString(", ")+additionalNode;
		d_out<<getNodeId(node)<<"[label=\""<<v<<"\""<<additionalNode<<"];\n";
		return true;
	}
	virtual void finish() {
		d_out<<"}\n";
		delete dot_out;
		dot_out=NULL;
		file->close();
		delete file;
		file=NULL;
	}
	virtual QString getExtraEdgeLayout(Node<NodeType, EdgeType> * node1, int edgeIndex) {return "";}
	virtual QString getExtraNodeLayout(Node<NodeType, EdgeType> * node) {return "";}
public:
	Tree2Dot(QString file):fileName(file) { this->file=NULL;dot_out=NULL;}
	#undef d_out
};


#endif // TREE2DOT_H
