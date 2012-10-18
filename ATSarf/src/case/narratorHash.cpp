#include "narratorHash.h"
#include "graph.h"

void NarratorHash::serialize(QDataStream & streamOut) {
	int size=hashTable.size();
	streamOut<<size;
	HashTable::iterator i=hashTable.begin();
	for (;i!=hashTable.end();i++){
		streamOut<<i.key();
		HashValue & v=i.value();
		streamOut<<graph->getSerializationNodeEquivalent(v.node);
		streamOut<<v.value
				 <<v.total;
	}
}
void NarratorHash::deserialize(QDataStream & streamIn) {
	int size;
	streamIn>>size;
	for (int i=0;i<size;i++){
		QString key;
		int nInt,value,total;
		streamIn>>key;
		streamIn>>nInt
				>>value
				>>total;
		NarratorNodeIfc * n=graph->getDeserializationIntEquivalent(nInt);
	#if 1
		if (n==NULL) {
			qDebug()<<key;
			continue;
		}
	#endif
		assert(n!=NULL);
		assert(n->isGroupNode());
		assert(n->size()>0);
		hashTable.insert(key,HashValue((GroupNode*)n,value,total));
	}
}
