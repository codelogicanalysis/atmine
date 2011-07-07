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
			return NULL;//(NarratorNodeIfc*)&nullGraphNarratorNode;
		} else {
			n=new GraphNarratorNode();
			chainIn>>n->indicies;
			n->deserializeHelper(chainIn,graph);
			return n;
		}
	} else {
		if (isNull) {
			return NULL;//(NarratorNodeIfc*)&nullChainNarratorNode;
		} else {
			n=new ChainNarratorNode();
			chainIn>>n->indicies;
			n->deserializeHelper(chainIn,graph);
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

	graphNode=NULL;//just for now, until we deserialize its equivalent graphNode

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
		NarratorNodeIfc * c=graph.getDeserializationIntEquivalent(cInt);
		assert(c!=NULL);
		assert(!c->isGraphNode());
	#if 1
		addNarrator(*(ChainNarratorNode*)c);
	#else
		ChainNarratorNode * c2=(ChainNarratorNode*)c;
		equalChainNodes.append(c2);
		c2->setCorrespondingNarratorNode(this);
	#endif
	}

	chainIn>>savedRank;

}
