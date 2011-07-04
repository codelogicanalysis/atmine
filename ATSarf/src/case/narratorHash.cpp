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
		ChainNarratorNode * n=graph->getDeserializationIntEquivalent(nInt);
		assert(n!=NULL);
		hashTable.insert(key,HashValue(n,value,total));
	}
}
