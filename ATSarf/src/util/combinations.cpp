#include "combinations.h"
#include <assert.h>

CombinationGenerator & CombinationGenerator::begin() {
	indicies.clear();
	if (isUnderDefined()) {
		indicies.append(-1); //i.e. finished
	} else {
		for (int i=0;i<numIndicies;i++) {
			indicies.append(i);
		}
	}
	return *this;
}

bool CombinationGenerator::initialize(int i, int index) {
	indicies[i]=index;
	for (int j=i+1;j<numIndicies;j++) {
		int newIndex=indicies[j-1]+1;
		if (newIndex<maxSize)
			indicies[j]=newIndex;
		else
			return false;
	}
	return true;
}

void CombinationGenerator::iterate(int i) {
	if (indicies.size()==0) {
		indicies.append(-1);
		return;
	}
	int & index=indicies[i];
	if (index+1<maxSize) {
		index++;
	} else {
		bool init=false;
		while (i>0) {
			i--;
			init=initialize(i,indicies[i]+1);
			if (init)
				break;
		}
		if (!init)
			indicies.last()=-1;

	}
}

CombinationGenerator & CombinationGenerator::operator++() {
	iterate(numIndicies-1);
	return *this;
}

const Combination & CombinationGenerator::getCombination() const {
	assert(numIndicies==indicies.size());
	return indicies;
}

bool CombinationGenerator::isFinished() const {
	return indicies.size()>0 && indicies.last()==-1;
}
