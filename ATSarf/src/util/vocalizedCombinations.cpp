#include "vocalizedCombinations.h"

VocalizedCombinationsGenerator::VocalizedCombinationsGenerator(QString voc, int numDiacritics) {
	this->voc=voc;
	for (int i=0;i<voc.size();i++) {
		QChar curr=voc[i];
		if (isDiacritic(curr)) {
			voc.remove(i,1);
			i--;
			diacritics.append(DiacPos(i,curr));
		}
	}
	unvoc=voc; //now all diacritics stripped out
	assert(unvoc==removeDiacritics(this->voc));
	this->numDiacritics=numDiacritics;
}
VocalizedCombinationsGenerator & VocalizedCombinationsGenerator::begin() {
	indicies.clear();
	if (isUnderVocalized()) {
		indicies.append(-1); //i.e. finished
	} else {
		for (int i=0;i<numDiacritics;i++) {
			indicies.append(i);
		}
	}
	return *this;
}

bool VocalizedCombinationsGenerator::initialize(int i, int index) {
	indicies[i]=index;
	for (int j=i+1;j<numDiacritics;j++) {
		int newIndex=indicies[j-1]+1;
		if (newIndex<diacritics.size())
			indicies[j]=newIndex;
		else
			return false;
	}
	return true;
}

void VocalizedCombinationsGenerator::iterate(int i) {
	if (indicies.size()==0) {
		indicies.append(-1);
		return;
	}
	int & index=indicies[i];
	int diacriticIndexSize=diacritics.size();
	if (index+1<diacriticIndexSize) {
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

VocalizedCombinationsGenerator & VocalizedCombinationsGenerator::operator++() {
	iterate(numDiacritics-1);
	return *this;
}

VocalizedCombination VocalizedCombinationsGenerator::getCombination() const {
	DiacriticsPositionsList diaSummary;
	QString s=unvoc;
	assert(numDiacritics==indicies.size());
	for (int i=numDiacritics-1;i>=0;i--) {
		int index=indicies[i];
		const DiacPos & d=diacritics[index];
		int pos=d.pos;
		QChar diacritic=d.diacritic;

		Diacritic dia=interpret_diacritic(diacritic);
		DiacriticsNposition diaNPos(pos,dia);
		diaSummary.prepend(diaNPos);

		s.insert(pos+1,diacritic);
	}
	return VocalizedCombination(s,diaSummary);
}

QString VocalizedCombinationsGenerator::getString() const {
	const VocalizedCombination & c=getCombination();
	return c.getString();
}

bool VocalizedCombinationsGenerator::isFinished() const {
	return indicies.size()>0 && indicies.last()==-1;
}

VocalizedCombination VocalizedCombination::deduceCombination(QString voc) {
	VocalizedCombination comb(voc,DiacriticsPositionsList());
	bool lastDiacritic=false;
	int shift=0;
	for (int i=0;i<voc.size();i++) {
		QChar c=voc[i];
		if (isDiacritic(c)) {
			Diacritic d=interpret_diacritic(c);
			comb.list.last().append(d);
			int pos;
			shift++;
			if (!lastDiacritic)
				pos=i-shift;
			else
				pos=comb.shortList.last().position;
			DiacriticsNposition s(pos,d);
			comb.shortList.append(s);
			lastDiacritic=true;
		} else {
			lastDiacritic=false;
			comb.list.append(Diacritics());
		}
	}
	return comb;
}

const QList<Diacritics> & VocalizedCombination::getDiacritics() {
	if (list.size()==0) {
		int pos=0;
		int shift=0;
		for (int i=0;i<shortList.size();i++) {
			shift++;
			bool lastDiacritic=(i>0 && shortList[i].position == shortList[i-1].position);
			int dia_pos_original=shortList[i].position;
			int dia_pos=dia_pos_original+shift;
			if (!lastDiacritic)
				pos++;
			for (;pos<dia_pos;pos++) {
				list.append(Diacritics());
			}
			assert(pos==dia_pos);
			QChar c=voc[pos];
			if (i>0 && lastDiacritic)
				list.last().append(c);
			else
				list.append(Diacritics(c));
			assert(interpret_diacritic(c)==shortList[i].diacritic);
			pos++;
		}
		for (;pos<voc.size();pos++) {
			list.append(Diacritics());
		}
	}
	assert(list.size()==voc.size()-shortList.size());
	return list;
}
