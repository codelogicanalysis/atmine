#ifndef MERGELISTS_H
#define MERGELISTS_H
/**
  * @file   mergeLists.h
  * @author Jad Makhlouta
  * @brief  This class defines a class and methods to merge lists
  */
#include <QList>

template <typename T>
class MergeLists {
private:
	#define MList QList
	#define List QList
	typedef int Index;
private:
	List<List<T> * > & lists;
	List<T> & result;

	MList<Index> indicies;

private:
	inline const T & at(int i) {
		return lists.at(i)->at(indicies[i]);
	}

	const T getSmallestAndProceed() {
		MList<int> smallestIteratorIndicies;
		T smallestValue=at(0);
		smallestIteratorIndicies.append(0);

		//find smallest
		for (int i=1;i<indicies.size();i++) {
			if (smallestValue>at(i)) {
				smallestIteratorIndicies.clear();
				smallestValue=at(i);
				smallestIteratorIndicies.append(i);
			} else if (smallestValue==at(i)) {
				smallestIteratorIndicies.append(i);
			}
		}

		MList<int> trash;

		//proceed
		for (int i=0;i<smallestIteratorIndicies.size();i++) {
			int listNumber=smallestIteratorIndicies[i];
			Index & itr=indicies[listNumber];
			Index max=lists[listNumber]->size();
			while (itr<max &&at(listNumber)==smallestValue ) {
				itr++;
			}
			if (itr==max) {
				trash.append(listNumber);
			}
		}

		//empty trash
		for (int i=trash.size()-1;i>=0;i--) { //delete from largest to smallest
			int listNumber=trash[i];
			delete lists[listNumber];
			lists.removeAt(listNumber);
			indicies.removeAt(listNumber);
		}
		return smallestValue;
	}
public:
	MergeLists(List<List<T> * > & allLists, List<T> & resultingList):lists(allLists), result(resultingList) {
		indicies.clear();
		int listNumber=lists.size();
		for (int i=0;i<listNumber;i++) {
			indicies.append(0);
		}
	}

	void mergeLists(  ) {
		while(lists.size()>1){
			result.append(getSmallestAndProceed());
		}
		if (lists.size()==1) {
			List<T> & list=*lists[0];
			Index itr=indicies[0];
			for (;itr<list.size();itr++) {
				if (itr ==0 || list[itr-1]!=list[itr])
					result.append(list[itr]);
			}
		}
	}
};

#endif // MERGELISTS_H
