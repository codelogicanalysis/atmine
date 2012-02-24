#include "ambiguity.h"


AmbiguitySolution::AmbiguitySolution(QString raw,QString des,QString POS): voc(raw), desc(des),pos(POS) {
	featuresDefined=false;
}

AmbiguitySolution::AmbiguitySolution(QString raw,QString des,QString POS, int stemStart, int suffStart, int stemIndex, int numPrefixes, int numSuffixes) {
	featuresDefined=true;
	voc=raw;
	desc=des;
	pos=POS;
	this->stemStart=stemStart;
	this->suffStart=suffStart;
	this->stemIndex=stemIndex;
	this->numPrefixes=numPrefixes;
	this->numSuffixes=numSuffixes;
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
			int index1=stemIndex;
			int index2=other.stemIndex;
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


item_types getDiacriticPosition(AmbiguitySolution & sol, int diacriticPos) {
	if (sol.featuresDefined) {
		if (diacriticPos>=sol.suffStart)
			return SUFFIX;
		else if (diacriticPos>=sol.stemStart)
			return STEM;
		else
			return PREFIX;
	} else {
		return STEM;
	}
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
	int stemStart=Stem->info.start;
	int suffStart=Suffix->info.start;
	int numPrefix=0,numSuff=0;
	int stemIndex;
	QString pos,desc,raw;
	for (int i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
			continue;
		desc+=pre.description()+" + ";
		pos+=pre.POS+"+";
		raw+=pre.raw_data;
		numPrefix++;
	}
	minimal_item_info & stem = *stem_info;
	desc+=stem.description()+" + ";
	pos+=stem.POS+"+";
	raw+=stem.raw_data;
	stemIndex=numPrefix;
	for (int i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
			continue;
		desc+=suff.description()+" + ";
		pos+=suff.POS+"+";
		raw+=suff.raw_data;
		numSuff++;
	}
	AmbiguitySolution s(raw,desc,pos,stemStart,suffStart,stemIndex,numPrefix,numSuff);
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
