#include <assert.h>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include "transliteration.h"
#include "stemmer.h"
#include "diacriticDisambiguation.h"
#include "vocalizedCombinations.h"




DiacriticDisambiguationBase::Solution::Solution(QString raw,QString des,QString POS): voc(raw), desc(des),pos(POS) {
	featuresDefined=false;
}

DiacriticDisambiguationBase::Solution::Solution(QString raw,QString des,QString POS, int stemStart, int suffStart, int stemIndex, int numPrefixes, int numSuffixes) {
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

QString DiacriticDisambiguationBase::Solution::getFeatureIndex(const QString & feature, int index) const {
	assert(index>=0);
	QStringList entries=feature.split("+");
	assert(index<entries.size());
	QString & r=entries[index];
	if (r[0]==' ')
		r.remove(0,1);
	if (r[r.size()-1]==' ')
		r.remove(r.size()-1,1);
	return r;
}

int DiacriticDisambiguationBase::Solution::getTokenization() const {
	return pos.count("PART")+pos.count("PREP")+pos.count("CONJ")+pos.count("DET")+pos.count("PRON")+pos.count("SUFF_DO");
}

bool DiacriticDisambiguationBase::Solution::equal (const DiacriticDisambiguationBase::Solution & other, Ambiguity m) const  {
	if (m==DiacriticDisambiguationBase::Vocalization) {
		return voc==other.voc;
	} else if (m==DiacriticDisambiguationBase::Description) {
		return desc==other.desc;
	} else if (m==DiacriticDisambiguationBase::POS) {
		return pos==other.pos;
	} else if (m==DiacriticDisambiguationBase::Stem_Ambiguity) {
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
	} else if (m==DiacriticDisambiguationBase::Tokenization) {
		int p1=getTokenization();
		int p2=other.getTokenization();
		return p1==p2;
	}
	return true;
}

void DiacriticDisambiguationBase::reset() {
	for (int i=0;i< ambiguitySize;i++) {
		total[i]=0;
		left[i]=0;
		leftBranching[i]=0;
		totalBranching[i]=0;
		countAmbiguity[i]=0;
		bestLeft[i]=0;
		worstLeft[i]=0;
		countReduced[i]=0;
		reducingCombinations[i]=0;
		totalCombinations[i]=0;
	}
	currId=-1;
	countWithoutDiacritics=0;
}

DiacriticDisambiguationBase::DiacriticDisambiguationBase(bool mapBased, bool suppressOutput, int diacriticsCount){
	this->mapBased=mapBased;
	this->suppressOutput=suppressOutput;
	this->diacriticsCount=diacriticsCount;
	reset();
}

void DiacriticDisambiguationBase::store(long id, QString entry, Solution & s) {
	if (mapBased) {
		AffixSolutionList & l=solutionMap[id];
		l.second.append(s);
		if (l.first.isEmpty())
			l.first=entry;
		/*else
			assert(entry==l.first);*/
	} else {
		if (currId!=id) {
			if (currId>=0) {
				analyze();
			}
			currId=id;
			currEntry=entry;
		}
		currSol.append(s);
	}
	assert(equal(s.voc,entry));
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS, int stemStart,
		   int suffStart, int stemIndex, int numPrefixes, int numSuffixes) {
	Solution s(raw_data,description,POS,stemStart,suffStart,stemIndex,numPrefixes,numSuffixes);
	store(id,entry,s);
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS) {
	Solution s(raw_data,description,POS);
	store(id,entry,s);
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
	case Stem_Ambiguity:
		return "Stem";
	case All_Ambiguity:
		return "All";
	default:
		return "---";
	}
}

item_types DiacriticDisambiguationBase::getDiacriticPosition(Solution & sol, int diacriticPos) const {
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

DiacriticDisambiguationBase::SolutionList DiacriticDisambiguationBase::getUnique(const SolutionList & list, Ambiguity m) {
	SolutionList l;
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

DiacriticDisambiguationBase::~DiacriticDisambiguationBase() {
	analyze();
	for (int amb=0;amb<ambiguitySize;amb++) {
		displayed_error	<<interpret((Ambiguity)amb)<<":"<<"\t"
						<<(double)(((long double)total[amb])/countAmbiguity[amb])<<" -->\t"
						<<(double)(left[amb]/countAmbiguity[amb])<<" =>\t"
						<<(long long)left[amb]<<"/"<<total[amb]<<" =\t"
						<<(double)(left[amb]/total[amb])<<",\t"
						<<(long long)leftBranching[amb]<<"/"<<totalBranching[amb]<<" =\t"
						<<(double)((long double)leftBranching[amb]/totalBranching[amb])<<"\t"
						<<"("<<bestLeft[amb]<<"/"<<total[amb]<<"="
						<<(double)(((long double)bestLeft[amb])/total[amb])<<","
						<<worstLeft[amb]<<"/"<<total[amb]<<"="
						<<(double)(((long double)worstLeft[amb])/total[amb])<<")\t"
						<<(double)((long double)countReduced[amb])/countAmbiguity[amb]<<"\t"
						<<(double)((long double)reducingCombinations[amb])/totalCombinations[amb]<<"\n";
	}
	displayed_error	<<"\nUndiacritized:\t"<<countWithoutDiacritics<<"/"<<countAmbiguity[All_Ambiguity]<<"=\t"
					<<(double)((long double)countWithoutDiacritics)/countAmbiguity[All_Ambiguity]<<"\n";
}

void DiacriticDisambiguationBase::analyze() {
	if (mapBased) {
		for (Map::iterator itr=solutionMap.begin();itr!=solutionMap.end();itr++) {
			QString currAffix=itr->first;
			SolutionList currSol=itr->second;
			analyzeOne(currAffix,currSol);
		}
		solutionMap.clear();
	} else {
		analyzeOne(currEntry,currSol);
		currId=-1;
		currEntry="";
		currSol.clear();
	}
}

void DiacriticDisambiguationBase::printDiacriticDisplay(Diacritics d) {
	if (!d.isSelfConsistent()) {
		out<<"~";
		return;
	}
	out<<(d.hasShadde()?1:0);
	out<<"-";
	Diacritic dia=d.getMainDiacritic();
	out<< (dia==UNDEFINED_DIACRITICS?0:(int)dia +1);
}

void DiacriticDisambiguationBase::printDiacritics(QString entry, int pos, QChar c) { //for one diacritic
	out	<<diacriticsCount<<"\t"<<entry.size();
	for (int p=0;p<pos;p++)
		out<<"\t"<<"0-0";
	Diacritics dia(c);
	out<<"\t";
	printDiacriticDisplay(dia);
	for (int p=pos+1;p<entry.size();p++)
		out<<"\t"<<"0-0";
	for (int p=entry.size();p<maxDiacritics;p++)
		out<<"\t"<<"X";
}

void DiacriticDisambiguationBase::printDiacritics(const QList<Diacritics> & d) { //for multiple diacritcs
	out	<<diacriticsCount<<"\t"<<d.size();
	for (int i=0;i<d.size();i++) {
		out<<"\t";
		printDiacriticDisplay(d[i]);
	}
	for (int i=d.size();i<maxDiacritics;i++)
		out<<"\t"<<"X";
}

void DiacriticDisambiguationBase::analyzeOne(QString currEntry,const SolutionList & currSol) {
	/*typedef QHash<QString, int> String2Int;
	typedef QPair<QString, int> StringIntPair;
	String2Int vocalizationIndicies;*/

	SolutionList currSolutions[ambiguitySize];
	QList<int> index[ambiguitySize];
	for (int amb=0;amb<ambiguitySize;amb++) {
		if ((Ambiguity)amb!=All_Ambiguity)
			currSolutions[amb]=getUnique(currSol,(Ambiguity)amb);
		else
			currSolutions[amb]=currSol;
	#if 0
		if (!suppressOutput)
			out<<currAffix<<": ";
	#endif
		for (int i=0;i<currSolutions[amb].size();i++) {
			index[amb].append(-1);
		#if 0
			if (!suppressOutput)
				out<<currSolutions[amb][i].voc<<", ";
		#endif
		}
	}
	/*for (int i=0;i<currSolutions[(int)Vocalization].size();i++) {
		vocalizationIndicies[currSolutions[(int)Vocalization][i].voc]=i;
	}*/
#if 0
	if (!suppressOutput)
		out<<"\n";
#endif


	int sub_total[ambiguitySize]={0}, sub_left[ambiguitySize]={0};
	//QList< QList<
	int best_sub_Left[ambiguitySize]={0}, worst_sub_Left[ambiguitySize]={0};
	for (int amb=0;amb<ambiguitySize;amb++) {
		best_sub_Left[amb]=currSolutions[amb].size();
	}



	if (diacriticsCount==1) {
		for (int i=-1;i<currEntry.size();i++) {
			int diacritics[ambiguitySize][(int)UNDEFINED_DIACRITICS+1]={0};
			int diacriticsSum[(int)UNDEFINED_DIACRITICS+1]={0};
			for (int amb=0;amb<ambiguitySize;amb++) {
				for (int j=0;j<currSolutions[amb].size();j++) {
					QString d=getDiacritics(currSolutions[amb][j].voc,index[amb][j]);
					if (d.size()>0 && !isDiacritic(d[0]))
						d.remove(0,1);
					assert(d.size()<=2);
					bool non_shadda=true;
					for (int k=0;k<d.size();k++) {
						QChar c=d[k];
						int c_d=(int)interpret_diacritic(c);
						if (c_d<UNDEFINED_DIACRITICS) {
							diacritics[amb][c_d]++;
							diacriticsSum[c_d]++;
							if (c_d!=(int)SHADDA)
								non_shadda=false;
						}
					}
					if (d.size()==0 || non_shadda) {
						diacritics[amb][(int)UNDEFINED_DIACRITICS]++;
						diacriticsSum[(int)UNDEFINED_DIACRITICS]++;
					}
				}
			}
			for (int j=0;j<UNDEFINED_DIACRITICS;j++) {
				if (diacriticsSum[j]==0)
					continue;
				double valid_ratio[ambiguitySize];

				for (int amb=0;amb<ambiguitySize;amb++) {
					int count_nothing=diacritics[amb][UNDEFINED_DIACRITICS];
					int count=diacritics[amb][j];
					int valid_count=(j==(int)SHADDA?count:count+count_nothing);
					valid_ratio[amb]=((double)valid_count)/(currSolutions[amb].size());

					worst_sub_Left[amb]=max(worst_sub_Left[amb],valid_count);
					best_sub_Left[amb]=min(best_sub_Left[amb],valid_count);
					sub_left[amb]+=valid_count;
					sub_total[amb]+=currSolutions[amb].size();

					if (valid_ratio[amb]<1)
						reducingCombinations[amb]++;
					totalCombinations[amb]++;
				}
				bool reduced=valid_ratio[All_Ambiguity]<1;
				bool display=!suppressOutput && reduced;
				if (display) {
					QChar c=interpret_diacritic((Diacritic)j);
					QString new_input=currEntry;
					new_input.insert(i+1,c);
					out	<<new_input<<"\t";
					printDiacritics(currEntry,i,c);
					for (int amb=0;amb<ambiguitySize;amb++) {
						out<<"\t"<<valid_ratio[amb];
					}
					out<<"\n";
				}
			}

		}
	} else {
		typedef QSet<VocalizedCombinations::Combination> CombSet;
		CombSet allPossibleComb;
		QSet<QString> vocs;
		for (int j=0;j<currSolutions[All_Ambiguity].size();j++) { //All_Ambiguity contains all solutions
			QString voc=currSolutions[All_Ambiguity][j].voc;
			if (diacriticsCount>1) {
				if (vocs.contains(voc))
					continue;
				vocs.insert(voc);
				VocalizedCombinations v(voc,diacriticsCount);
				if (v.isUnderVocalized())
					allPossibleComb.insert(VocalizedCombinations::Combination::deduceCombination(voc));
				else {
					for (v.begin();!v.isFinished();++v) {
						allPossibleComb.insert(v.getCombination());
					}
				}
			} else { //i.e. all diacritics
				allPossibleComb.insert(VocalizedCombinations::Combination::deduceCombination(voc));
			}
		}
		CombSet::iterator itr=allPossibleComb.begin();
		for (;itr!=allPossibleComb.end();itr++) {
			VocalizedCombinations::Combination & c=(VocalizedCombinations::Combination &)(*itr);
			QString s=c.getString();
			const QList<Diacritics> & d=c.getDiacritics();
			int numDia=c.getNumDiacritics();
			if (numDia==0)
				continue;
			double valid_ratio[ambiguitySize];
			for (int amb=0;amb<ambiguitySize;amb++) {
				int valid_count=0;
				for (int j=0;j<currSolutions[amb].size();j++) {
					if (equal(currSolutions[amb][j].voc,s))
						valid_count++;
				}
				valid_ratio[amb]=((double)valid_count)/(currSolutions[amb].size());
				sub_left[amb]+=valid_count;
				worst_sub_Left[amb]=max(worst_sub_Left[amb],valid_count);
				best_sub_Left[amb]=min(best_sub_Left[amb],valid_count);
				sub_total[amb]+=currSolutions[amb].size();

				if (valid_ratio[amb]<1)
					reducingCombinations[amb]++;
				totalCombinations[amb]++;
			}
			bool reduced=valid_ratio[All_Ambiguity]<1;
			bool display=((numDia==diacriticsCount|| diacriticsCount==-1) && !suppressOutput && reduced);
			if (display) {
				out	<<s<<"\t";
				printDiacritics(d);
				for (int amb=0;amb<ambiguitySize;amb++) {
					out<<"\t"<<valid_ratio[amb];
				}
				out<<"\n";
			}
		}
	}
	for (int amb=0;amb<ambiguitySize;amb++) {
		if (sub_total[amb]==0) {
		#if 0
			left[amb]+=currSolutions[amb].size();
			worstLeft[amb]+=currSolutions[amb].size();
			bestLeft[amb]+=currSolutions[amb].size();
			//leftBranching[amb]+=0;
			totalBranching[amb]+=sub_total[amb];
			total[amb]+=currSolutions[amb].size();
		#else
			if (amb==(int)All_Ambiguity)
				countWithoutDiacritics++;
		#endif
		} else {
			left[amb]+=((double)sub_left[amb])/sub_total[amb]*currSolutions[amb].size();
			worstLeft[amb]+=worst_sub_Left[amb];
			bestLeft[amb]+=best_sub_Left[amb];
			leftBranching[amb]+=sub_left[amb];
			totalBranching[amb]+=sub_total[amb];
			total[amb]+=currSolutions[amb].size();
		}
		assert(totalBranching[amb]>0);
		if (currSolutions[amb].size()!=0) {
			countAmbiguity[amb]++;
			if (sub_left[amb]<sub_total[amb])
				countReduced[amb]++;
		}
	}
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
	storage.store(id,info.getString(),raw,desc,pos,stemStart,suffStart,stemIndex,numPrefix,numSuff);
	return true;
}

FullDisambiguation::FullDisambiguation(QString inputFileName, ATMProgressIFC * prg, int numDiacritcs, QString outputFileName): DiacriticDisambiguationBase(false,true, numDiacritcs) {
	this->inputFileName=inputFileName;
	this->outputFileName=outputFileName;
	this->prg=prg;
	if (!outputFileName.isEmpty()) {
		suppressOutput=false;
	}
}

void FullDisambiguation::operator()() {
	if (!outputFileName.isEmpty()) {
		QFile::remove(outputFileName);
		outFile.setFileName(outputFileName);
		assert(outFile.open(QFile::ReadWrite));
		oldDevice=out.device();
		out.setDevice(&outFile);
		out.setCodec("utf-8");
	}
	QFile input(inputFileName);
	if (!input.open(QIODevice::ReadOnly)) {
		out << "File not found\n";
		return;
	}
	QTextStream file(&input);
	//file.setCodec("utf-8");
	long count=0;
	long size=input.size();
	long pos=0;
	while (!file.atEnd()) {
		QString line=file.readLine(0);
	#if 0
		//qDebug()<<line;
		QStringList entries=line.split('\t',QString::SkipEmptyParts);
		if (entries.size()!=2)
			continue;
		assert(entries.size()==2);
		QString unvoc=Buckwalter::convertFrom(entries[0]);
		QString voc=Buckwalter::convertFrom(entries[1]);
		if (!equal(voc,unvoc))
			continue;
	#else
		if (line=="")
			continue;
		QString unvoc=Buckwalter::convertFrom(line);
	#endif
		DisambiguationStemmer s(count,unvoc,*this);
		s();
		//analyze();
		count++;
		pos+=line.size()+1;//to account for '\n'
		prg->report((((double)pos)/size)*100+0.5);
	}
	input.close();
}

FullDisambiguation::~FullDisambiguation() {
	analyze();
	if (outFile.isOpen())
		outFile.close();
	out.setDevice(oldDevice);
}


void diacriticDisambiguationCount(item_types t, int numDiacritics=1) {
	if (t==STEM) {
		StemDisambiguation d(numDiacritics);
		d();
	} else {
		AffixDisambiguation d(t,numDiacritics);
		d();
	}
}

void diacriticDisambiguationCount(QString fileName, int numDiacritics, ATMProgressIFC * prg, QString outputFile="fullOutput") {
	FullDisambiguation d(fileName, prg, numDiacritics,outputFile);
	d();
}
