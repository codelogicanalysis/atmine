#include <assert.h>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include "transliteration.h"
#include "stemmer.h"
#include "morphemes.h"
#include "diacriticDisambiguation.h"
#include "vocalizedCombinations.h"

typedef QSet<VocCombIndexListPair> CombSet;
typedef QPair<QString,MorphemeDiacritics> TextNmorphDiaPair;

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

void DiacriticDisambiguationBase::store(long id, QString entry, AmbiguitySolution & s) {
	//assert(equal(s.voc,entry));
	if (!equal(s.voc,entry)) {
		qDebug()<<"Conflict:\t"<<s.voc<<entry;
		return;
	}
	if (mapBased) {
		EntryAmbiguitySolutionList & l=solutionMap[id];
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

}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS, Morphemes morphemes) {
	AmbiguitySolution s(raw_data,description,POS,morphemes);
	store(id,entry,s);
}

void DiacriticDisambiguationBase::store(long id, QString entry, QString raw_data, QString description, QString POS) {
	AmbiguitySolution s(raw_data,description,POS);
	store(id,entry,s);
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
			AmbiguitySolutionList currSol=itr->second;
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

void DiacriticDisambiguationBase::printDiacriticDisplay(Diacritics d, QTextStream * o) {
#ifdef ALL_DIA
	if (!d.isSelfConsistent()) {
		(*o)<<"~"<<diacritic_shadde_delimeter<<"~";
		return;
	}
	(*o)<<(d.hasShadde()?1:0);
	(*o)<<diacritic_shadde_delimeter;
	Diacritic dia=d.getMainDiacritic();
	(*o)<< (dia==UNDEFINED_DIACRITICS?0:(int)dia +1);
#else
	if (!d.isSelfConsistent()) {
		(*o)<<"~";
	} else if (d.hasShadde()) {
		(*o)<<(int)SHADDA;
	} else {
		Diacritic dia=d.getMainDiacritic();
		if (dia!=UNDEFINED_DIACRITICS)
			(*o)<<(int)dia;
	}
#endif
}

void DiacriticDisambiguationBase::printDiacritics(QString entry, int pos, QChar c, QTextStream * o) { //for one diacritic
	(*o)<<entry.size()<<"\t"<<diacriticsCount;
#ifdef ALL_DIA
	for (int p=0;p<pos;p++)
		(*o)<<"\t"<<"0"<<diacritic_shadde_delimeter<<"0";
	Diacritics dia(c);
	(*o)<<"\t";
	printDiacriticDisplay(dia,o);
	for (int p=pos+1;p<entry.size();p++)
		(*o)<<"\t"<<"0"<<diacritic_shadde_delimeter<<"0";
	for (int p=entry.size();p<maxDiacritics;p++)
		(*o)<<"\t"<<"X"<<diacritic_shadde_delimeter<<"X";
#else
	Diacritics dia(c);
	(*o)<<"\t"<<pos<<"\t";
	printDiacriticDisplay(dia,o);
#endif
}

void DiacriticDisambiguationBase::printDiacritics(QString unvoc,const QList<Diacritics> & d, AmbiguitySolution sol, QTextStream * o) { //for multiple diacritcs
	(*o)<<d.size()<<"\t"<<diacriticsCount;
#if ALL_DIA
	for (int i=0;i<d.size();i++) {
		(*o)<<"\t";
		printDiacriticDisplay(d[i],o);
	}
	for (int i=d.size();i<maxDiacritics;i++)
		(*o)<<"\t"<<"X"<<diacritic_shadde_delimeter<<"X";
#else
	for (int i=0;i<d.size();i++) {
		const Diacritics & dia=d[i];
		if (!dia.isEmpty()) {
			(*o)<<"\t"<<i<<"\t";
			printDiacriticDisplay(dia,o);
			int relPos, morphSize;
			MorphemeType t=sol.getMorphemeTypeAtPosition(i,dia,relPos,morphSize);
			QChar letterBefore=unvoc[i];
			(*o)<<"\t"<<(int)t
				<<"\t"<<relPos
				<<"\t"<<getRelativePos(relPos,morphSize)
				<<"\t"<<letterBefore
				<<"\t"<<isLongVowel(letterBefore)
				<<"\t"<<isShamsi(letterBefore);
		}
	}
#endif
}


void insertIntoCombSet(QSet<TextNmorphDiaPair> & alreadyProceesed,CombSet & allPossibleComb,VocalizedCombination & comb,int index, AmbiguitySolution & sol) {
	QString s=comb.getString();
	MorphemeDiacritics mD=sol.getMorphemeDiacriticSummary(comb);
	TextNmorphDiaPair p(s,mD);
	if (alreadyProceesed.contains(p))
		return;
	alreadyProceesed.insert(p);
	CombSet::iterator itr=allPossibleComb.find(comb);
	if (itr==allPossibleComb.end()) {
		VocCombIndexListPair p(comb);
		p.indicies.insert(index);
		allPossibleComb.insert(p);
	} else {
		VocCombIndexListPair & p=(VocCombIndexListPair &)(*itr); //safe since changing p by adding index does not disrupt its position in set
		p.indicies.insert(index);
	}
}


void DiacriticDisambiguationBase::analyzeOne(QString currEntry,const AmbiguitySolutionList & currSol) {
	AmbiguitySolutionList currSolutions[ambiguitySize];
#ifdef ONE_SPECIAL
	QList<int> index[ambiguitySize];
#endif
	for (int amb=0;amb<ambiguitySize;amb++) {
		if ((Ambiguity)amb!=All_Ambiguity)
			currSolutions[amb]=getAmbiguityUnique(currSol,(Ambiguity)amb);
		else
			currSolutions[amb]=currSol;
	#ifdef ONE_SPECIAL
		for (int i=0;i<currSolutions[amb].size();i++) {
			index[amb].append(-1);
		}
	#endif
	}


	int sub_total[ambiguitySize]={0}, sub_left[ambiguitySize]={0};
	int best_sub_Left[ambiguitySize]={0}, worst_sub_Left[ambiguitySize]={0};
	for (int amb=0;amb<ambiguitySize;amb++) {
		best_sub_Left[amb]=currSolutions[amb].size();
	}


#ifdef ONE_SPECIAL
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
#else
	{
#endif
		CombSet allPossibleComb;
		QSet<TextNmorphDiaPair> alreadyProceesed;
		for (int j=0;j<currSolutions[All_Ambiguity].size();j++) { //All_Ambiguity contains all solutions
			AmbiguitySolution & sol=currSolutions[All_Ambiguity][j];
			QString voc=sol.voc;
			if (diacriticsCount>0) {
				VocalizedCombinationsGenerator v(voc,diacriticsCount);
				if (v.isUnderVocalized()) {
					VocalizedCombination c=VocalizedCombination::deduceCombination(voc);
					insertIntoCombSet(alreadyProceesed,allPossibleComb,c,j,sol);
				} else {
					for (v.begin();!v.isFinished();++v) {
						VocalizedCombination c=v.getCombination();
						insertIntoCombSet(alreadyProceesed,allPossibleComb,c,j,sol);
					}
				}
			} else { //i.e. all diacritics
				VocalizedCombination c=VocalizedCombination::deduceCombination(voc);
				insertIntoCombSet(alreadyProceesed,allPossibleComb,c,j,sol);
			}
		}
		CombSet::iterator itr=allPossibleComb.begin();
		for (;itr!=allPossibleComb.end();itr++) {
			VocCombIndexListPair & combIndexList=(VocCombIndexListPair &)(*itr);
			VocalizedCombination & c=combIndexList.comb;
			QString s=c.getString();
			const QList<Diacritics> & d=c.getDiacritics();
			//const DiacriticsPositionsList & diaPos=c.getShortList();
			int numDia=c.getNumDiacritics();
			if (numDia==0)
				continue;
			double valid_ratio[ambiguitySize];
			for (int amb=0;amb<ambiguitySize;amb++) {
				int valid_count=0;
				for (int j=0;j<currSolutions[amb].size();j++) {
					if (equal(s,currSolutions[amb][j].voc,true))
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
			for (int i=0;i<2;i++) {
				QTextStream * o=(i==0?&out:&hadith_out);
				if (display || i==1) { //always display for hadith_out
					for (QSet<int>::iterator itr=combIndexList.indicies.begin();itr!=combIndexList.indicies.end();++itr) {
						int index=*itr;
						AmbiguitySolution & sol =currSolutions[All_Ambiguity][index]; //make instead of indicies in general to indicies to uniques ones out of All_Ambiguity
						(*o)<<s<<"\t";
						printDiacritics(currEntry,d,sol,o);
						for (int amb=0;amb<ambiguitySize;amb++) {
							(*o)<<"\t"<<valid_ratio[amb];
						}
						(*o)<<"\n";
					}
				}
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
			if (amb==(int)All_Ambiguity && !currEntry.isEmpty())
				countWithoutDiacritics++;
		#endif
		} else {
			/*if (diacriticsCount<0) {
				qDebug()<<sub_total[amb]<<" vs "<<currSolutions[amb].size();
				assert(sub_total[amb]==currSolutions[amb].size());
			}*/
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

void DisambiguationStemmer::store(QString entry,AmbiguitySolution & s) {
	storage.store(id,entry,s);
}

FullListDisambiguation::FullListDisambiguation(QStringList & inputList, ATMProgressIFC * prg, int numDiacritcs): DiacriticDisambiguationBase(false,true, numDiacritcs), list(inputList) {
	this->prg=prg;
}

void FullListDisambiguation::operator()() {
	QString unvoc;
	int count=0;
	long size=list.size();
	foreach(unvoc, list) {
		DisambiguationStemmer s(count,unvoc,*this);
		s();
		count++;
		prg->report((((double)count)/size)*100+0.5);
	}
}

FullListDisambiguation::~FullListDisambiguation() {
	analyze();
}

FullFileDisambiguation::FullFileDisambiguation(QString inputFileName, ATMProgressIFC * prg, int numDiacritcs, QString reducedFileName, QString allFileName): DiacriticDisambiguationBase(false,true, numDiacritcs) {
	this->inputFileName=inputFileName;
	this->reducedFileName=reducedFileName;
	this->allFileName=allFileName;
	this->prg=prg;
	if (!allFileName.isEmpty() || !reducedFileName.isEmpty()) {
		suppressOutput=false;
	}
}

void FullFileDisambiguation::operator()() {
	if (!reducedFileName.isEmpty()) {
		QFile::remove(reducedFileName);
		reducedFile.setFileName(reducedFileName);
		assert(reducedFile.open(QFile::ReadWrite));
		oldDevice=out.device();
		out.setDevice(&reducedFile);
		out.setCodec("utf-8");
	}
	if (!allFileName.isEmpty()) {
		QFile::remove(allFileName);
		allFile.setFileName(allFileName);
		assert(allFile.open(QFile::ReadWrite));
		oldDeviceAll=hadith_out.device();
		hadith_out.setDevice(&allFile);
		hadith_out.setCodec("utf-8");
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

FullFileDisambiguation::~FullFileDisambiguation() {
	analyze();
	if (reducedFile.isOpen())
		reducedFile.close();
	out.setDevice(oldDevice);
	if (allFile.isOpen())
		allFile.close();
	hadith_out.setDevice(oldDeviceAll);
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

void diacriticDisambiguationCount(QString fileName, int numDiacritics, ATMProgressIFC * prg, QString reducedFile="reducedOutput", QString allFile="fullOutput") {
	FullFileDisambiguation d(fileName, prg, numDiacritics,reducedFile,allFile);
	d();
}

void diacriticDisambiguationCount(QStringList & list, int numDiacritics, ATMProgressIFC * prg) {
	FullListDisambiguation d(list, prg, numDiacritics);
	d();
}

