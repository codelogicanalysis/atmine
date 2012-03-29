#ifndef MORPHEMES_H
#define MORPHEMES_H

#include <QString>
#include <QPair>
#include <assert.h>
#include "common.h"


enum MorphemeType {Proc3, Proc2, Proc1, Proc0, Prefix, Stem, Suffix, Enc0, Invalid_morpheme};

const int morpheme_bits_size=6;
extern int bit_PROC3, bit_PROC2, bit_PROC1, bit_PROC0, bit_DET, bit_ENC0;
extern int * morpheme_bits[morpheme_bits_size];
static const QString morpheme_abstract_desc[morpheme_bits_size]={"proc3", "proc2", "proc1", "proc1", "det", "enc0"};
static const MorphemeType morpheme_types[morpheme_bits_size]={Proc3, Proc2, Proc1, Proc0, Proc0, Enc0}; //TODO: make det


class Morpheme{
public:
	int start:14;
	int end:14;
	MorphemeType type:4;
public:
	Morpheme(int aStart=-1, int aEnd=-1, MorphemeType aType=Invalid_morpheme):start(aStart),end(aEnd),type(aType) {}
	item_types getItemType() const {
		switch(type) {
		case Proc3:
		case Proc2:
		case Proc1:
		case Proc0:
		case Prefix:
			return PREFIX;
		case Stem:
			return STEM;
		case Suffix:
		case Enc0:
			return SUFFIX;
		default:
			return ITEM_TYPES_LAST_ONE;
		}
	}
	int size() const { assert(end>=start);return end-start+1;}
	bool operator==(const Morpheme & m ) const {return start==m.start && end==m.end && type==m.type;}
	void setType(dbitvec abstract_categories, item_types itemType) {
		typedef QPair<int, int> IndexPair;
		IndexPair range[3]={IndexPair(0,4), //Prefix
							IndexPair(0,-1), //Stem => check nothing
							IndexPair(5,5)}; //Suffix
		bool typeSet=false;
		if (itemType==STEM) {//check nothing
			type=Stem;
			typeSet=true;
		} else {
			assert(itemType!=ITEM_TYPES_LAST_ONE);
			IndexPair & r=range[(int)itemType];
			for (int i=r.first;i<=r.second;i++) {
				if (abstract_categories.getBit(*morpheme_bits[i])) {
					type= morpheme_types[i];
					typeSet=true;
				}
			}
			if (!typeSet) {
				type =(itemType==PREFIX?Prefix:Suffix);
			}

		}
	}
};
class Morphemes  {
private:
	QList<Morpheme> list;

	int stemIndex:6;
	int prefixCount:6;
	int suffixCount:6;
public:
	Morphemes() {
		stemIndex=-1;
		prefixCount=0;
		suffixCount=0;
	}
	int getStemIndex() const { return stemIndex; }
	int getAffixCount(item_types type) const {
	#if 0
		int count=0;
		for (int i=0;i<size();i++) {
			if (at(i).getItemType()==type)
				count++;
		}
		return count;
	#else
		switch (type) {
		case PREFIX:
			return prefixCount;
		case STEM:
			return stemIndex>=0;
		case SUFFIX:
			return suffixCount;
		default:
			return 0;
		}
		return 0;
	#endif
	}
	void append(Morpheme & m) { //assumes appended in correct order
		item_types type=m.getItemType();
		switch (type) {
		case PREFIX:
			prefixCount++;
			break;
		case STEM:
			stemIndex=size();
			break;
		case SUFFIX:
			suffixCount++;
			break;
		default:
			break;
		}
		list.append(m);
	}
	bool operator ==(const Morphemes & m) const { return list==m.list; }
	Morpheme & operator [](int i) { return list[i];}
	const Morpheme & operator [](int i) const { return list[i];}
	int size() const {return list.size(); }
};

void morpheme_initialize();


#endif // MORPHEMES_H
