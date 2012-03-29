#include "ambiguity.h"


AmbiguitySolution::AmbiguitySolution(QString raw,QString des,QString POS): voc(raw), desc(des),pos(POS) {
	featuresDefined=false;
}

AmbiguitySolution::AmbiguitySolution(QString raw,QString des,QString POS, Morphemes morphemes) {
	featuresDefined=true;
	voc=raw;
	desc=des;
	pos=POS;
	this->morphemes=morphemes;
}

QString AmbiguitySolution::getFeatureIndex(const QString & feature, int index) const {
	assert(index>=0);
	QStringList entries=feature.split("+");
	assert(index<entries.size());
	QString & r=entries[index];
	if (r.size()>0 && r[0]==' ')
		r.remove(0,1);
	if (r.size()>0 && r[r.size()-1]==' ')
		r.remove(r.size()-1,1);
	return r;
}

int AmbiguitySolution::getTokenization() const {
	return pos.count("PART")+pos.count("PREP")+pos.count("CONJ")+pos.count("DET")+pos.count("PRON")+pos.count("SUFF_DO");
}

bool AmbiguitySolution::equal (const AmbiguitySolution & other, Ambiguity m) const  {
	if (m==Vocalization) {
		return voc==other.voc;
	} else if (m==Description) {
		return desc==other.desc;
	} else if (m==POS) {
		return pos==other.pos;
	} else if (m==Stem_Ambiguity) {
		if (featuresDefined) {
			int index1=morphemes.getStemIndex();
			int index2=other.morphemes.getStemIndex();
			QString pos1=getFeatureIndex(pos,index1);
			QString pos2=getFeatureIndex(other.pos,index2);
			QString desc1=getFeatureIndex(desc,index1);
			QString desc2=getFeatureIndex(other.desc,index2);
			return pos1==pos2 && desc1==desc2;
		} else
			return false;
	} else if (m==Tokenization) {
		int p1=getTokenization();
		int p2=other.getTokenization();
		return p1==p2;
	}
	return true;
}

QString interpret(Ambiguity a) {
	switch (a) {
	case Vocalization:
		return "VOC";
	case Tokenization:
		return "Token";
	case Description:
		return "Gloss";
	case POS:
		return "POS";
	case Stem_Ambiguity:
		return "Stem";
	case All_Ambiguity:
		return "All";
	default:
		return "---";
	}
}


MorphemeType AmbiguitySolution::getMorphemeTypeAtPosition(int & diacriticPos, const QList<int> & diaPos, int & relativePos, int & morphemeSize) {
	//TODO: check if works correctly for more than one diacritic
	int diaPosTemp=diacriticPos; //the loop below performed to make diaPos consistent with position in word with diacritics and not just position of letter (assuming no diacritics)
	for (int i=0;i<diaPos.size();i++) { //can be made faster
		if (diacriticPos<=diaPos[i])
			diaPosTemp++;
	}
	diacriticPos=diaPosTemp;
	if (featuresDefined) {
		for (int i=morphemes.size()-1;i>=0;i--) {
			Morpheme & m=morphemes[i];
			int relPos=diacriticPos-m.start;
			if (relPos>=0) {
				relativePos=relPos;
				morphemeSize=m.size();
				return m.type;
			}
		}
	}
	relativePos=diacriticPos;
	morphemeSize=voc.size(); //not very correct depends on degree of vocalization of the input, but best approximation we can do for now. anyways we must not reach this line in our application
	return Stem;
}

AmbiguitySolutionList getAmbiguityUnique(const AmbiguitySolutionList & list, Ambiguity m) {
	AmbiguitySolutionList l;
	for (int i=0;i<list.size();i++) {
		int j;
		for (j=0;j<l.size();j++) {
			if (l.at(j).equal(list.at(i),m)) {
				break;
			}
		}
		if (j==l.size())
			l.append(list.at(i));
	}
	return l;
}

bool AmbiguityStemmerBase::on_match() {
	Morphemes morphemes;
	QString pos,desc,raw;
	int last=0;
	for (int i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
			continue;
		desc+=pre.description()+" + ";
		pos+=pre.POS+"+";
		raw+=pre.raw_data;
		int current=Prefix->sub_positionsOFCurrentMatch[i];
		Morpheme m(last,current);
		m.setType(pre.abstract_categories,PREFIX);
		morphemes.append(m);
		last=current;
	}
	minimal_item_info & stem = *stem_info;
	desc+=stem.description()+" + ";
	pos+=stem.POS+"+";
	raw+=stem.raw_data;
	int current=Stem->info.finish;
	Morpheme m(last,current,::Stem);
	morphemes.append(m);
	last=current;

	for (int i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
			continue;
		desc+=suff.description()+" + ";
		pos+=suff.POS+"+";
		raw+=suff.raw_data;
		int current=Suffix->sub_positionsOFCurrentMatch[i];
		Morpheme m(last,current);
		m.setType(suff.abstract_categories,SUFFIX);
		morphemes.append(m);
		last=current;
	}
	AmbiguitySolution s(raw,desc,pos,morphemes);
	store(info.getString(),s);
	return true;
}

void AmbiguityStemmer::store(QString /*entry*/, AmbiguitySolution & s) {
	list.append(s);
}

int AmbiguityStemmer::getAmbiguity(Ambiguity amb) const {
	if (amb==All_Ambiguity)
		return list.size();
	AmbiguitySolutionList l=getAmbiguityUnique(list,amb);
	return l.size();

}
