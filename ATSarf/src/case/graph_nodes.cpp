#include "graph_nodes.h"
#include "narrator_abstraction.h"
#include "graph.h"

#define min(a,b) a>b?b:a

NULLChainNarratorNode nullChainNarratorNode;
NULLNarratorNodeIfc nullNarratorNodeIfc;
NULLGraphNarratorNode nullGraphNarratorNode;
ChainsContainer chains;
NodeAddress nullNodeAddress((NarratorNodeIfc &)nullNarratorNodeIfc,(ChainNarratorNode &)nullChainNarratorNode);
ChainNodeIterator ChainNodeIterator::null(NULL,-1,-1);

NarratorNodeIfc::NarratorNodeIfc(NarratorGraph & g)
	: id(g.nodesCount){
	resetColor();
	g.addNode(this);
}


ChainNodeIterator NarratorNodeIfc::begin() {
	return ChainNodeIterator(this,0,0);
}

NarratorNodeIfc & ChainNarratorNode::getCorrespondingNarratorNode()
{
	if (group!=NULL)
		return (*group).getCorrespondingNarratorNode();
	else
		return (*this);
}

ChainNarratorNode & ChainNarratorNode::prevInChain()
{
	return (previous!=NULL
				?(*previous)
				:(ChainNarratorNode &)nullChainNarratorNode);
}

ChainNarratorNode & ChainNarratorNode::nextInChain()
{
	return (next!=NULL
				?(*next)
				:(ChainNarratorNode &)nullChainNarratorNode);
}

int ChainNarratorNode::getSavedRank() const {
	return (group==NULL?savedRank:group->getSavedRank());
}
void ChainNarratorNode::setRank(int rank) {
	savedRank=rank;
	if(group!=NULL)
		group->setRank(rank);
}

void GroupNode::setRank(int rank) {
	assert(graphNode!=NULL);
	graphNode->setRank(rank);
}

int GroupNode::getSavedRank() const {
	assert(graphNode!=NULL);
	return graphNode->getSavedRank();
}

NarratorNodeIfc & GroupNode::getCorrespondingNarratorNode() {
	//assert(graphNode!=NULL);
	return *graphNode;
}

NarratorNodeIfc * NarratorNodeIfc::deserialize(QDataStream &chainIn, NarratorGraph & graph) {
	bool isNull,isGraph,isChain;
	chainIn >>isNull
			>>isGraph
			>>isChain;
	if (isNull)
		return NULL;
	NarratorNodeIfc * n;
	if (isGraph) {
		n=new GraphNarratorNode();
	} else if (isChain) {
		n=new ChainNarratorNode();
	} else {
		n=new GroupNode();
	}
#if 0
	n->setId(graph.nodesCount);
	graph.addNode(n);
#endif
	chainIn>>n->indicies;
	n->deserializeHelper(chainIn,graph);
	return n;
}

void ChainNarratorNode::serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const {
	int pInt=graph.allocateSerializationNodeEquivalent(previous),
		nInt=graph.allocateSerializationNodeEquivalent(next);
	assert(pInt>0 || nInt>0); //at least one not null
	chainOut<<pInt
			<<nInt;

	chainOut<<graph.getHadithStringSerializationEquivalent(narrator->hadith_text);
	narrator->serialize(chainOut);

	chainOut<<savedRank;

	chainOut<<chainContext.getChainNum()
			<<chainContext.getIndex();


}
void ChainNarratorNode::deserializeHelper(QDataStream &streamIn,NarratorGraph & graph) {
	int pInt,nInt;
	streamIn >>pInt
			>>nInt;
	NarratorNodeIfc * p=(pInt==0?NULL:graph.getDeserializationIntEquivalent(pInt)),
					* n=(nInt==0?NULL:graph.getDeserializationIntEquivalent(nInt));
	assert (p==NULL || !p->isGraphNode());
	assert (n==NULL || !n->isGraphNode());
	previous=(ChainNarratorNode*)p;
	next=(ChainNarratorNode*)n;
	if (previous!=NULL)
		previous->next=this;
	if (next!=NULL)
		next->previous=this;

	group=NULL;//just for now, until we deserialize its equivalent graphNode

	int hadithString;
	streamIn>>hadithString;
	QString *text=graph.getHadithStringDeserializationIntEquivalent(hadithString);
	assert(text!=NULL);
	narrator=new Narrator(text); //TODO: check for not resulting in memory leaks
	qint8 c;
	streamIn>>c;
	assert(c=='a');
	narrator->deserialize(streamIn);

	streamIn>>savedRank;

	int chainNum,index;
	streamIn>>chainNum
			>>index;

	chainContext.set(index,chainNum);
}

void GraphNarratorNode::serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const {
	int size=groupList.size();
	chainOut<<size;
	for (int i=0;i<size;i++) {
		GraphNodeItem & c=(*groupList[i]);
		int cInt=graph.allocateSerializationNodeEquivalent(&c);
		assert(cInt>0); //not null
		chainOut<<cInt;
		c.serialize(chainOut,graph);
	}

	chainOut<<savedRank;

}
void GraphNarratorNode::deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) {
	groupList.clear();
	int size,cInt;
	chainIn >>size;
	for (int i=0;i<size;i++) {
		chainIn>>cInt;
		assert(cInt>0); //not null
		NarratorNodeIfc * c=NarratorNodeIfc::deserialize(chainIn,graph);
		graph.setDeserializationIntEquivalent(cInt,c);
		assert(c!=NULL);
		assert(!c->isGraphNode());
		GroupNode* g=(GroupNode*)c;
		groupList.append(g);
		g->setGraphNode(this);
	}

	chainIn>>savedRank;

}

void GroupNode::serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const {
	int s=size();
	chainOut<<s;
	for (int i=0;i<s;i++) {
		ChainNarratorNode & c=(*list[i]);
		int cInt=graph.allocateSerializationNodeEquivalent(&c);
		assert(cInt>0); //not null
		chainOut<<cInt;
		c.serialize(chainOut,graph);
	}

	chainOut<<key;

}
void GroupNode::deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) {
	list.clear();
	int size,cInt;
	chainIn >>size;
	for (int i=0;i<size;i++) {
		chainIn>>cInt;
		assert(cInt>0); //not null
		NarratorNodeIfc * c=NarratorNodeIfc::deserialize(chainIn,graph);
		graph.setDeserializationIntEquivalent(cInt,c);
		assert(c!=NULL);
		assert(!c->isGraphNode());
		ChainNarratorNode* n=(ChainNarratorNode*)c;
		list.append(n);
		n->setCorrespondingNarratorNodeGroup(this);
	}

	chainIn>>key;
	setGraphNode(NULL);

}

NarratorNodeIfc & ChainNarratorNode::getChild(int index1,int index2) {
	assert(index1==0 && index2==0);
	if (isLast())
		return (NarratorNodeIfc &)nullChainNarratorNode;
	else
		return nextInChain().getCorrespondingNarratorNode();
}
NarratorNodeIfc & ChainNarratorNode::getParent(int index1,int index2)
{
	assert(index1==0 && index2==0);
	if (isFirst())
		return (NarratorNodeIfc &)nullChainNarratorNode;
	else
		return prevInChain().getCorrespondingNarratorNode();
}

ChainNarratorNode & ChainNarratorNode::getChainNodeInChain(int chain_num) 	{
	if (getChainNum()==chain_num)
		return *this;
	else
		return (ChainNarratorNode &)nullChainNarratorNode;
}
