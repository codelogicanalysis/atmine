#include <QFile>
#include <QStringList>
#include <QTextStream>
#include "atbDiacriticExperiment.h"
#include "vocalizedCombinations.h"
#include "transliteration.h"

void AmbCombStat::reset(QString voc) {
	this->voc=voc;
	this->bestComb=voc;
	this->worstComb=voc;
	this->bestAmbiguity=500;
	this->worstAmbiguity=0;
	this->countComb=0;
	this->totalAmbiguity=0;
}

void AmbCombStat::augmentStats(QString comb, int ambiguity) {
	if (ambiguity>worstAmbiguity) {
		worstAmbiguity=ambiguity;
		worstComb=comb;
	}
	if (ambiguity<bestAmbiguity) {
		bestAmbiguity=ambiguity;
		bestComb=comb;
	}
	totalAmbiguity+=ambiguity;
	countComb++;
}

AmbCombStat::AmbCombStat(QString voc, QString bestComb, QString worstComb, int bestAmbiguity, int worstAmbiguity,
					  int countComb, int totalAmbiguity) {
	this->voc=voc;
	this->bestComb=bestComb;
	this->worstComb=worstComb;
	this->bestAmbiguity=bestAmbiguity;
	this->worstAmbiguity=worstAmbiguity;
	this->countComb=countComb;
	this->totalAmbiguity=totalAmbiguity;
}

int addRandomDiacritics(QString & voc, int num_voc) {
	QList<int> diacritics;
	for (int i=0;i<voc.size();i++) {
		QChar curr=voc[i];
		if (isDiacritic(curr))
			diacritics.append(i);
	}
	for (int i=0;i<num_voc;i++) {
		int d_size=diacritics.size();
		if (d_size==0)
			return i;
		int index=rand()%d_size;
		diacritics.removeAt(index);
	}
	for (int i=diacritics.size()-1;i>=0;i--) {
		voc.remove(diacritics[i],1);
	}
	return num_voc;
}

void getStatDiacriticAssignmentHelper( QString s, AmbiguityStatList & stats) {
	AmbiguityStemmer stemmer(s);
	stemmer();
	for (int amb=0;amb<ambiguitySize;amb++) {
		int ambiguity=stemmer.getAmbiguity((Ambiguity)amb);
		stats[amb].augmentStats(s,ambiguity);
	}
}

void getStatDiacriticAssignment( QString voc, int numDiacritics, AmbiguityStatList & stats) {
	stats.clear();
	for (int i=0;i<ambiguitySize;i++)
		stats.append(AmbCombStat(voc));
	VocalizedCombinationsGenerator c(voc,numDiacritics);
	if (c.isUnderVocalized()) {
	#ifdef MAX_FOR_UNDERVOCALIZED
		QString s=voc;
		getStatDiacriticAssignmentHelper(s,stats);
	#endif
	} else {
		for (c.begin();!c.isFinished();++c) {
			QString s=c.getString();
			getStatDiacriticAssignmentHelper(s,stats);
		}
	}
}

#define Has_Tanween (partial_voc.contains(fathatayn) && c==1)

int atbDiacritic(QString inputString, ATMProgressIFC *prg) {
	//TODO: apply affect for tanween in equality if present force it

	QFile diacritics_file(inputString);
	if (!diacritics_file.open(QIODevice::ReadWrite) ) {
		out << "Diacritics File not found\n";
		return 1;
	}
	QString line;
	int total=0;
	const int maxDiacritics=6;
	int no_voc_total[ambiguitySize]={0},voc_total[ambiguitySize]={0};
	int voc_ambiguity[maxDiacritics][ambiguitySize]={0};
	int no_voc_ambiguity[maxDiacritics][ambiguitySize]={0};
	int total_count[maxDiacritics][ambiguitySize]={0};
	int full_vocalized[ambiguitySize]={0};
#ifdef RANDOM_DIACRITICS
	int random_voc_ambiguity[maxDiacritics][ambiguitySize]={0};
	int random_total_count[maxDiacritics][ambiguitySize]={0};
	int random_full_vocalized[ambiguitySize]={0};
#else
	double other_ambiguity[maxDiacritics][ambiguitySize]={0};
	int other_best_ambiguity[maxDiacritics][ambiguitySize]={0};
	int other_worst_ambiguity[maxDiacritics][ambiguitySize]={0};
	int other_count[maxDiacritics][ambiguitySize]={0};

	int partial_best[maxDiacritics][ambiguitySize]={0};
	int partial_worst[maxDiacritics][ambiguitySize]={0};
	double partial_average[maxDiacritics][ambiguitySize]={0};
	int partial_best_total[ambiguitySize]={0};
	int partial_worst_total[ambiguitySize]={0};
	double partial_average_total[ambiguitySize]={0};
	int correctly_detected[ambiguitySize]={0}, correctly_detectedNoDiacritics[ambiguitySize]={0};
#endif
	int countReduced=0, countEquivalent=0, countEquivalentTanween=0, countReducedTanween=0, countTanween=0;
	QTextStream file(&diacritics_file);
	int filePos=0;
#ifdef THEORETICAL_DIACRITICS
	long fileSize=diacritics_file.size();
#else
	long fileSize=file.readAll().size();
	file.seek(0);
#endif
	while (!file.atEnd() ) {
		line=file.readLine(0);
		filePos+=line.size()+1;
		if (line.startsWith("Diacritics") || line.startsWith("ERROR") || line.isEmpty())
			continue;
		total++;
		QStringList entries= line.split("\t",QString::KeepEmptyParts);
	#ifndef THEORETICAL_DIACRITICS
		QString voc=entries[0];
		QString partial_voc=entries[1];
		QString no_voc=removeDiacritics(partial_voc);
		int c=entries[2].toInt();
	#else
		QString voc=Buckwalter::convertFrom(entries[0]);
		QString no_voc=Buckwalter::convertFrom(entries[1]);
	#endif
		QString ignore;
	#ifdef RECALL_DIACRITICS
		ignore=(entries.size()==4?entries[3]:"");
		if(!equal(voc,no_voc))
			continue;
	#endif
	#ifdef THEORETICAL_DIACRITICS
		{
			for (int amb=0;amb<ambiguitySize;amb++) {
				{
	#else
	#ifndef CHECK_ALL
		if (equal(partial_voc,voc,true)) {
	#endif
			AmbiguityStemmer stemmer(partial_voc);
			stemmer();
			AmbiguityStemmer stemmer2(no_voc);
			stemmer2();
			AmbiguityStemmer stemmer3(voc);
			stemmer3();

			for (int amb=0;amb<ambiguitySize;amb++) {
				Ambiguity ambiguity=(Ambiguity)amb;
				int voc_amb=stemmer.getAmbiguity(ambiguity);
				int no_voc_amb=stemmer2.getAmbiguity(ambiguity);
				int full_amb=stemmer3.getAmbiguity(ambiguity);
				no_voc_total[amb]+=no_voc_amb;
				voc_total[amb]+=voc_amb;
				no_voc_ambiguity[c][amb]+=no_voc_amb;
				voc_ambiguity[c][amb]+=voc_amb;
				total_count[c][amb]++;
				if (!partial_voc.contains(fathatayn) && c==1) {
					no_voc_ambiguity[0][amb]+=no_voc_amb;
					voc_ambiguity[0][amb]+=voc_amb;
					total_count[0][amb]++;
				}

				full_vocalized[amb]+=full_amb;

				if (ambiguity==All_Ambiguity) {
					QTextStream * print;
					if (no_voc_amb!=voc_amb) {
						print=&out;
						countReduced++;
						if (Has_Tanween) {
							countReducedTanween++;
						}
					} else {
						print=&displayed_error;
					}
				#ifndef RECALL_DIACRITICS
					(*print)<<partial_voc<<"\t"<<no_voc_amb<<"\t"<<voc_amb<<"\n";
				#endif
				}
	#ifdef RECALL_DIACRITICS
		if (equal(no_voc,voc))
			correctly_detectedNoDiacritics[amb]++;
		else if (ambiguity==All_Ambiguity)
			displayed_error<<no_voc<<"\t"<<voc<<"\n";
		if (ignore=="i") {
			correctly_detected[amb]++;
			assert(!equal(partial_voc,voc,true));
		}
	#endif
	#ifdef CHECK_ALL
		if (equal(partial_voc,voc,true)) {
			correctly_detected[amb]++;
	#endif
	#endif
			#ifdef RANDOM_DIACRITICS
				//random voc
				random_voc_ambiguity[0][amb]+=no_voc_amb;
				random_total_count[0][amb]++;
				for (int i=1;i<maxDiacritics;i++) {
					QString v=voc;
					/*if (*/addRandomDiacritics(v,i);/*==i) {*/
						AmbiguityStemmer stemmer(v);
						stemmer();
						int a=stemmer.getAmbiguity(ambiguity);
						random_voc_ambiguity[i][amb]+=a;
						random_total_count[i][amb]++;
					//}
				}
				random_full_vocalized[amb]+=full_amb;
			#else
			#ifndef THEORETICAL_DIACRITICS
				if (ambiguity==All_Ambiguity) {
			#endif
					for (int i=0;i<maxDiacritics;i++) {
						AmbiguityStatList d;
						getStatDiacriticAssignment(voc,i,d);
						for (int amb=0;amb<ambiguitySize;amb++) {
							if (d[amb].countComb>0) {
								double average=((double)d[amb].totalAmbiguity)/d[amb].countComb;
								int best=d[amb].bestAmbiguity;
								int worst=d[amb].worstAmbiguity;
								other_ambiguity[i][amb]+=average;
								other_best_ambiguity[i][amb]+=best;
								other_worst_ambiguity[i][amb]+=worst;
								other_count[i][amb]++;
							#ifndef THEORETICAL_DIACRITICS
								if (c==i ) {
									partial_average[i][amb]+=average;
									partial_best[i][amb]+=best;
									partial_worst[i][amb]+=worst;
									partial_average_total[amb]+=average;
									partial_best_total[amb]+=best;
									partial_worst_total[amb]+=worst;

									if (!partial_voc.contains(fathatayn) && c==1) {
										partial_average[0][amb]+=average;
										partial_best[0][amb]+=best;
										partial_worst[0][amb]+=worst;
									}
								}
							#endif
							}
						}
					}
				}

			#endif
			}
			countEquivalent++;
		#ifndef THEORETICAL_DIACRITICS
			if (Has_Tanween) {
				countEquivalentTanween++;
			}
		#endif

		}
	#ifndef THEORETICAL_DIACRITICS
		if (Has_Tanween)
			countTanween++;
	#endif
                if(prg != NULL)
                    prg->report(((double)filePos)/fileSize*100+0.5);
	}
	for (int amb=0;amb<ambiguitySize;amb++) {
		displayed_error<<interpret((Ambiguity)amb)<<":\n";
#ifndef RECALL_DIACRITICS
	#ifdef CHECK_ALL
		int total_equivalent=total;
	#else
		int total_equivalent=countEquivalent;
	#endif
		double full_voc_ratio=((double)full_vocalized[amb])/total_equivalent;
	#ifndef THEORETICAL_DIACRITICS
		for (int i=0;i<maxDiacritics;i++) {
			double	voc_ratio=((double)voc_ambiguity[i][amb])/total_count[i][amb],
					no_voc_ratio=((double)no_voc_ambiguity[i][amb])/total_count[i][amb],
					average_ratio=((double)partial_average[i][amb])/total_count[i][amb],
					best_ratio=((double)partial_best[i][amb])/total_count[i][amb],
					worst_ratio=((double)partial_worst[i][amb])/total_count[i][amb];
			QString num=QString("%1").arg(i);
			if (i==0) {
				int t=total_count[1][amb]-total_count[0][amb];
				double	voc_ratio1=((double)voc_ambiguity[1][amb]-voc_ambiguity[0][amb])/t,
						no_voc_ratio1=((double)no_voc_ambiguity[1][amb]-no_voc_ambiguity[0][amb])/t,
						average_ratio1=((double)partial_average[1][amb]-partial_average[0][amb])/t,
						best_ratio1=((double)partial_best[1][amb]-partial_best[0][amb])/t,
						worst_ratio1=((double)partial_worst[1][amb]-partial_worst[0][amb])/t;
				num=QString("1 ( ")+fathatayn+"  )";
				displayed_error	<<"\tDiacritics\t"<<num<<"\t"<<no_voc_ratio1<<"\t"<<voc_ratio1
								<<"->\t"<<average_ratio1<<"\t("<<best_ratio1<<",\t"<<worst_ratio1<<")\n";

				num=QString("1 (- ")+fathatayn+"  )";
			}
			displayed_error	<<"\tDiacritics\t"<<num<<"\t"<<no_voc_ratio<<"\t"<<voc_ratio
							<<"->\t"<<average_ratio<<"\t("<<best_ratio<<",\t"<<worst_ratio<<")\n";
		}
		double	voc_ratio=((double)voc_total[amb])/total_equivalent,
				no_voc_ratio=((double)no_voc_total[amb])/total_equivalent,
				average_ratio=((double)partial_average_total[amb])/total_equivalent,
				best_ratio=((double)partial_best_total[amb])/total_equivalent,
				worst_ratio=((double)partial_worst_total[amb])/total_equivalent;
		displayed_error	<<"\tDiacritics\t+\t"<<no_voc_ratio<<"\t"<<voc_ratio
						<<"->\t"<<average_ratio<<"\t("<<best_ratio<<",\t"<<worst_ratio<<")\n";

		int t=total_equivalent+total_count[0][amb]-total_count[1][amb];
		double	voc_ratio1=((double)voc_total[amb]+voc_ambiguity[0][amb]-voc_ambiguity[1][amb])/t,
				no_voc_ratio1=((double)no_voc_total[amb]+no_voc_ambiguity[0][amb]-no_voc_ambiguity[1][amb])/t,
				average_ratio1=((double)partial_average_total[amb]+partial_average[0][amb]-partial_average[1][amb])/t,
				best_ratio1=((double)partial_best_total[amb]+partial_best[0][amb]-partial_best[1][amb])/t,
				worst_ratio1=((double)partial_worst_total[amb]+partial_worst[0][amb]-partial_worst[1][amb])/t;
		displayed_error	<<"\tDiacritics\t+ (- "<<fathatayn<<" )\t"<<no_voc_ratio1<<"\t"<<voc_ratio1
						<<"->\t"<<average_ratio1<<"\t("<<best_ratio1<<",\t"<<worst_ratio1<<")\n";

		displayed_error	<<"\tDiacritics\t*\t"<<full_voc_ratio<<"\n\n\n";

	#endif
	#ifdef RANDOM_DIACRITICS
		for (int i=0;i<maxDiacritics;i++) {
			double	ratio=((double)random_voc_ambiguity[i][amb])/random_total_count[i][amb];
			displayed_error<<"\tDiacritics\t"<<i<<"\t"<<ratio<<"\n";
		}
		double	random_full_voc_ratio=((double)random_full_vocalized[amb])/random_total_count[0][amb];
		displayed_error<<"\tDiacritics\t*\t"<<random_full_voc_ratio<<"\n";
	#else
		for (int i=0;i<maxDiacritics;i++) {
			double	ratio=((double)other_ambiguity[i][amb])/other_count[i][amb];
			double	best_ratio=((double)other_best_ambiguity[i][amb])/other_count[i][amb];
			double	worst_ratio=((double)other_worst_ambiguity[i][amb])/other_count[i][amb];
			displayed_error<<"\tDiacritics\t"<<i<<"\t"<<ratio<<"\t("<<best_ratio<<",\t"<<worst_ratio<<")\n";
		}
		displayed_error<<"\tDiacritics\t*\t"<<full_voc_ratio<<"\n";
	#endif
#else
		displayed_error	<<"\tRecall=\t"<<correctly_detected[amb]<<"/"<<correctly_detectedNoDiacritics[amb]<<"=\t"<<correctly_detected[amb]/((double)correctly_detectedNoDiacritics[amb])<<"\n"
						<<"\tPrecision=\t"<<correctly_detected[amb]<<"/"<<voc_total[amb]<<"=\t"<<correctly_detected[amb]/((double)voc_total[amb])<<"\n\n";
		displayed_error	<<"\tRecall (no)=\t"<<correctly_detectedNoDiacritics[amb]<<"/"<<correctly_detectedNoDiacritics[amb]<<"=\t"<<1.0<<"\n"
						<<"\tPrecision (no)=\t"<<correctly_detectedNoDiacritics[amb]<<"/"<<no_voc_total[amb]<<"=\t"<<correctly_detectedNoDiacritics[amb]/((double)no_voc_total[amb])<<"\n\n";
#endif
	}
	displayed_error<<"\nEquivalent Factor:\t"<<countEquivalent<<"/"<<total<<"=\t"<<((double)countEquivalent)/total<<"\n";
	displayed_error<<"Reduction Factor:\t"<<countReduced<<"/"<<countEquivalent<<"=\t"<<((double)countReduced)/countEquivalent<<"\n";
	displayed_error<<"\nEquivalent Tanween Factor:\t"<<countEquivalentTanween<<"/"<<countTanween<<"=\t"<<((double)countEquivalentTanween)/countTanween<<"\n";
	displayed_error<<"Reduction Tanween Factor:\t"<<countReducedTanween<<"/"<<countEquivalentTanween<<"=\t"<<((double)countReducedTanween)/countEquivalentTanween<<"\n";
	int countNonReduced=countReduced-countReducedTanween;
	int countNonEquivalent=countEquivalent-countEquivalentTanween;
	displayed_error<<"Reduction Non-Tanween Factor:\t"<<countNonReduced<<"/"<<countNonEquivalent<<"=\t"<<((double)countNonReduced)/countNonEquivalent<<"\n";
	return 0;
}

