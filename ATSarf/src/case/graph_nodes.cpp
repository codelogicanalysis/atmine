#include "graph_nodes.h"
#include "narrator_abstraction.h"
#include "graph.h"

#define min(a,b) a>b?b:a

NULLChainNarratorNode nullChainNarratorNode;
NULLNarratorNodeIfc nullNarratorNodeIfc;
NULLGraphNarratorNode nullGraphNarratorNode;
ChainsContainer chains;
NodeAddress nullNodeAddress((NarratorNodeIfc &)nullNarratorNodeIfc,(ChainNarratorNode &)nullChainNarratorNode);

NarratorNodeIfc & ChainNarratorNode::getCorrespondingNarratorNode()
{
	if (graphNode!=NULL)
		return (*graphNode);
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

NarratorNodeIfc * NarratorNodeIfc::deserialize(QDataStream &chainIn, NarratorGraph & graph) {
	bool isNull,isGraph;
	chainIn >>isNull
			>>isGraph;
	NarratorNodeIfc * n;
	if (isGraph) {
		if (isNull) {
			return (NarratorNodeIfc*)&nullGraphNarratorNode;
		} else {
			n=new GraphNarratorNode();
			chainIn>>n->indicies;
			n->serializeHelper(chainIn,graph);
			return n;
		}
	} else {
		if (isNull) {
			return (NarratorNodeIfc*)&nullChainNarratorNode;
		} else {
			n=new ChainNarratorNode();
			chainIn>>n->indicies;
			n->serializeHelper(chainIn,graph);
			return n;
		}
	}
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
void ChainNarratorNode::deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) {
	int pInt,nInt;
	chainIn >>pInt
			>>nInt;
	previous=graph.getDeserializationIntEquivalent(pInt);
	next=graph.getDeserializationIntEquivalent(nInt);
	if (previous!=NULL)
		previous->next=this;
	if (next!=NULL)
		next->previous=this;

	graphNode=NULL;//just for now, until we deserialize its equivalent graphNode

	int hadithString;
	chainIn>>hadithString;
	QString *text=graph.getHadithStringDeserializationIntEquivalent(hadithString);
	assert(text!=NULL);
	narrator=new Narrator(text); //TODO: check for not resulting in memory leaks
	narrator->deserialize(chainIn);

	chainIn>>savedRank;

	int chainNum,index;
	chainIn	>>chainNum
			>>index;
	chainContext.set(index,chainNum);
}

void GraphNarratorNode::serializeHelper(QDataStream &chainOut, NarratorGraph & graph) const {
	int size=equalChainNodes.size();
	chainOut<<size;
	for (int i=0;i<size;i++) {
		int cInt=graph.allocateSerializationNodeEquivalent(equalChainNodes[i]);
		assert(cInt>0); //not null
		chainOut<<cInt;
	}

	chainOut<<savedRank;

}
void GraphNarratorNode::deserializeHelper(QDataStream &chainIn,NarratorGraph & graph) {
	equalChainNodes.clear();
	int size,cInt;
	chainIn >>size;
	for (int i=0;i<size;i++) {
		chainIn>>cInt;
		assert(cInt>0); //not null
		ChainNarratorNode * c=graph.getDeserializationIntEquivalent(cInt);
		assert(c!=NULL);
		addNarrator(*c);
	}

	chainIn>>savedRank;

}
