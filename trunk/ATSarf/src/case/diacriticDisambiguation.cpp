#include <assert.h>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include "transliteration.h"
#include "stemmer.h"
#include "diacriticDisambiguation.h"


DiacriticDisambiguationBase::DiacriticDisambiguationBase(){
	for (int i=0;i<=(int)All_Ambiguity;i++) {
		total[i]=0;
		left[i]=0;
		countAmbiguity[i]=0;
	}
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS) {
	AffixSolutionList & l=solutionMap[id];
	Solution s(raw_data,description,POS);
	l.second.append(s);
	if (l.first.isEmpty())
		l.first=entry;
	/*else
		assert(entry==l.first);*/
	assert(equal(raw_data,entry));
}

int DiacriticDisambiguationBase::Solution::getTokenization() const {
	return pos.count("PART")+pos.count("PREP")+pos.count("CONJ")+pos.count("PRON");
}

bool DiacriticDisambiguationBase::Solution::equal (const DiacriticDisambiguationBase::Solution & other, Ambiguity m) const  {
	if (m==DiacriticDisambiguationBase::Vocalization) {
		return voc==other.voc;
	} else if (m==DiacriticDisambiguationBase::Description) {
		return desc==other.desc;
	} else if (m==DiacriticDisambiguationBase::POS) {
		return pos==other.pos;
	} else if (m==DiacriticDisambiguationBase::Tokenization) {
		int p1=getTokenization();
		int p2=other.getTokenization();
		return p1==p2;
	}
	return true;
}

QString DiacriticDisambiguationBase::interpret(Ambiguity a) {
	switch (a) {
	case Vocalization:
		return "VOC";
	case Tokenization:
		return "Token";
	case Description:
		return "Gloss";
	case POS:
		return "POS";
	case All_Ambiguity:
		return "ALL";
	default:
		return "---";
	}
}

DiacriticDisambiguationBase::SolutionList DiacriticDisambiguationBase::getUnique(const SolutionList & list, Ambiguity m) {
	SolutionList l;
	for (int i=0;i<list.size();i++) {
		int j;
		for (j=0;j<l.size();j++) {
			if (l.at(j).equal(list.at(i),m))
				break;
		}
		if (j==l.size())
			l.append(list.at(i));
	}
	return l;
}

DiacriticDisambiguationBase::~DiacriticDisambiguationBase() {
	analyze();
	for (int amb=0;amb<=(int)All_Ambiguity;amb++) {
		displayed_error	<<interpret((Ambiguity)amb)<<":"<<"\t"<<left[amb]
						<<"/"<<total[amb]<<"\t"<<left[amb]/total[amb]<<"\t"
						<<left[amb]/countAmbiguity[amb]<<"\t"<<((double)total[amb])/countAmbiguity[amb]<<"\n";
	}
}

void DiacriticDisambiguationBase::analyze() {
	for (int amb=0;amb<=(int)All_Ambiguity;amb++) {
		out<<"\n"<<interpret((Ambiguity)amb)<<":\n";
		for (Map::iterator itr=solutionMap.begin();itr!=solutionMap.end();itr++) {
			QString currAffix=itr->first;
			SolutionList currSolutions=itr->second;
			if ((Ambiguity)amb!=All_Ambiguity)
				currSolutions=getUnique(currSolutions,(Ambiguity)amb);
			QList<int> index;
			out<<currAffix<<": ";
			for (int i=0;i<currSolutions.size();i++) {
				index.append(-1);
				out<<currSolutions[i].voc<<", ";
			}
			total[amb]+=currSolutions.size();
			out<<"\n";
			int t=0, l=0;
			for (int i=-1;i<currAffix.size();i++) {
				int diacritics[(int)UNDEFINED_DIACRITICS+1]={0};
				for (int j=0;j<currSolutions.size();j++) {
					QString d=getDiacritics(currSolutions[j].voc,index[j]);
					if (d.size()>0 && !isDiacritic(d[0]))
						d.remove(0,1);
					assert(d.size()<=2);
					bool non_shadda=true;
					for (int k=0;k<d.size();k++) {
						QChar c=d[k];
						int c_d=(int)interpret_diacritic(c);
						if (c_d<UNDEFINED_DIACRITICS) {
							diacritics[c_d]++;
							if (c_d!=(int)SHADDA)
								non_shadda=false;
						}
					}
					if (d.size()==0 || non_shadda)
						diacritics[(int)UNDEFINED_DIACRITICS]++;
				}
				int dia_nothing=diacritics[UNDEFINED_DIACRITICS];
				for (int j=0;j<UNDEFINED_DIACRITICS;j++) {
					QChar c=interpret_diacritic((Diacritic)j);
					QString new_input=currAffix;
					new_input.insert(i+1,c);
					int count=diacritics[j];
					if (count==0)
						continue;
					int valid_count=(j==(int)SHADDA)?count:count+dia_nothing;
					double valid_ratio=((double)valid_count)/currSolutions.size();
					out<<"\t"<<new_input<<"\t"<<valid_ratio<<"\n";
					l+=valid_count;
					t+=currSolutions.size();
				}
			}
			if (t==0)
				left[amb]+=currSolutions.size();
			else
				left[amb]+=((double)l)/t*currSolutions.size();
			countAmbiguity[amb]++;
		}
	}
	solutionMap.clear();
}

void AffixDisambiguation::visit(node *n, QString affix, QString raw_data, long /*category_id*/, QString description, QString POS) {
	store((long)n,affix,raw_data,description,POS);
}

void StemDisambiguation::operator()() {
	for (int i=0;i<stemNodes->size();i++) {
		StemNode & n=(*stemNodes)[i];
		long stem_id=n.stem_id;
		StemNode::CategoryVector & cats=n.category_ids;
		for (int c=0;c<cats.size();c++) {
			long cat_id=cats[c];
			//QString cat=database_info.comp_rules->getCategoryName(cat_id);
			StemNode::RawDatasEntry & raws=n.raw_datas[c];
			for (int r=0;r<raws.size();r++) {
				QString raw=raws[r];
				QString stem=removeDiacritics(raw); //bc currently n.key left empty
				ItemEntryKey entry(stem_id,cat_id,raw);
				ItemCatRaw2AbsDescPosMapItr itr=map->find(entry);
				while (itr!=map->end() && itr.key()==entry) {
					//dbitvec d=itr.value().first;
					QString pos=itr.value().third;
					long desc_id=itr.value().second;
					QString desc;
					if (desc_id>=0)
						desc=(*database_info.descriptions)[desc_id];
					store(stem_id,stem,raw,desc,pos);
					itr++;
				}
			}
		}
	}
}

bool DisambiguationStemmer::on_match() {
	QString pos,desc,raw;
	for (int i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
			continue;
		desc+=pre.description()+" + ";
		pos+=pre.POS+"+";
		raw+=pre.raw_data;
	}
	minimal_item_info & stem = *stem_info;
	desc+=stem.description()+" + ";
	pos+=stem.POS+"+";
	raw+=stem.raw_data;
	for (int i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
			continue;
		desc+=suff.description()+" + ";
		pos+=suff.POS+"+";
		raw+=suff.raw_data;
	}
	storage.store(id,info.getString(),raw,desc,pos);
	return true;
}

void FullDisambiguation::operator()() {
	QFile input(fileName);
	if (!input.open(QIODevice::ReadOnly)) {
		out << "File not found\n";
		return;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");
	long count=0;
	long size=input.size();
	while (!file.atEnd()) {
		QString line=file.readLine();
		qDebug()<<line;
		QStringList entries=line.split('\t',QString::SkipEmptyParts);
		/*if (entries.size()!=2)
			continue;*/
		assert(entries.size()==2);
		QString unvoc=Buckwalter::convertFrom(entries[0]);
		QString voc=Buckwalter::convertFrom(entries[1]);
		assert(equal(voc,unvoc));
		DisambiguationStemmer s(count,unvoc,*this);
		s();
		analyze();
		count++;
		long pos=file.pos();
		prg->report((((double)pos)/size)*100+0.5);
	}
	input.close();
}


void diacriticDisambiguationCount(item_types t) {
	if (t==STEM) {
		StemDisambiguation d;
		d();
	} else {
		AffixDisambiguation d(t);
		d();
	}
}

void diacriticDisambiguationCount(QString fileName, ATMProgressIFC * prg) {
	FullDisambiguation d(fileName, prg);
	d();
}
