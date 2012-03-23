#include "atbExperiment.h"
#include <cstdlib>
#include <time.h>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <assert.h>
#include "logger.h"
#include "stemmer.h"
#include "transliteration.h"
#include "diacritics.h"


#ifdef SAVE_CONFLICTS
	#undef TOKENIZE
	#undef SPECIAL_TOKENIZE
	#undef READ_CONFLICTS
#endif

AtbStemmer::Status AtbStemmer::updateSimilarFields(Status oldStat, Status currentStat, QString currGloss, QString currVoc, int old_pos) {
	assert(oldStat!=D_ALL);
	if (currentStat==D_ALL) {
		if (oldStat==D_GL)
			similarGlosses.removeLast();
		else if (oldStat==D_VOC)
			similarVoc.removeLast();
		return D_ALL;
	} else if (oldStat==D_GL && currentStat==D_VOC) {
		similarGlosses.removeLast();
		oldStat=EQ; //so that we fill missing voc
	} else if (oldStat==D_VOC && currentStat==D_GL) {
		currentStat=EQ; //so that we append current currGloss
	}


	if (oldStat==EQ && currentStat==EQ) {
		return EQ;
	} else if (oldStat==D_VOC && (currentStat==D_VOC ||currentStat==EQ)) {
		similarVoc.last()+="+"+currVoc;
		return D_VOC;
	} else if (oldStat==EQ && currentStat==D_VOC){
		QString s;
		for (int i=0;i<old_pos;i++) {
			s+=voc[i]+"+";
		}
		s+=currVoc;
		similarVoc.append(s);
		return D_VOC;
	} else if (oldStat==D_GL && (currentStat==D_GL ||currentStat==EQ)) {
		similarGlosses.last()+=" + "+currGloss;
		return D_GL;
	} else if (oldStat==EQ && currentStat==D_GL){
		QString s;
		for (int i=0;i<old_pos;i++) {
			s+=gloss[i]+" + ";
		}
		s+=currGloss;
		similarGlosses.append(s);
		return D_GL;
	}
	return D_ALL;
}

AtbStemmer::Status AtbStemmer::equal(int & index,minimal_item_info & item, Status currentStat, bool ignore) {
	if (index>=voc.size())
		return D_ALL;
	QString v, g,p;
	int count=item.POS.count("+");
	bool inconsistent_pos=false;
	if (count>0) {
		QStringList parts=item.POS.split("+");
		for (int i=0;i<parts.size();i++) {
			QStringList entries=parts[i].split("/");
			assert(entries.size()==2);
			v+=entries[0];
			p=entries[1];
			int j=index+i;
			if (j>=pos.size() || p!=pos[j])
				inconsistent_pos=true;
		}
	} else {
		QStringList entries=item.POS.split("/");
		assert(entries.size()==2);
		v=entries[0];
		p=entries[1];
		if (pos[index]!=p)
			inconsistent_pos=true;
	}
	int j=index;
	index+=count+1;
	g=item.description();
	Status s;
	if (inconsistent_pos && !ignore) {
		s=D_ALL;
	} else if (ignore) {
		s=EQ;
	} else {
		QString v_atb,g_atb;
		for (int i=j;i<index;i++) {
			v_atb+=voc[i];
			g_atb+=gloss[i];
			if (i!=index-1)
				g_atb+=" + ";
		}
	#if 0
		g.replace("with/by","by/with");
		g.replace("for/to","to/for");
		g.replace("it/him","him/it");
		g.replace("it/he","he/it");
		g.replace("so/and","and/so");
		g.replace("as/like","like/such as");
		g_atb.replace("with/by","by/with");
		g_atb.replace("for/to","to/for");
		g_atb.replace("it/him","him/it");
		g_atb.replace("it/he","he/it");
		g_atb.replace("so/and","and/so");
		g_atb.replace("as/like","like/such as");
	#endif
		g.replace(" + ","+");
		g_atb.replace(" + ","+");

		bool eq_v=(v_atb==v),
			 eq_g=(g_atb==g);
		if (!eq_v && eq_g)
			s=D_VOC;
		else if (!eq_g && eq_v)
			s=D_GL;
		else if (!eq_g && !eq_v)
			s=D_ALL;
		else
			s=EQ;
	}
	return updateSimilarFields(currentStat,s,g,v,j);
}

AtbStemmer::AtbStemmer(QString & input,const QStringList &aVoc, QStringList &aGloss, const QStringList & aPOS, QStringList &aInputAfter, const QStringList & aPOSAfter, bool ignoreStem)
			:Stemmer(&input,0),voc(aVoc),gloss(aGloss), pos(aPOS), input_after(aInputAfter), pos_after(aPOSAfter) 	{
	assert(voc.size()>=gloss.size());
	while(voc.size()>gloss.size())
		gloss.append("");

	assert(pos.size()==voc.size());
	this->ignoreStem=ignoreStem;
	found=false;
	stat=D_ALL;
	correctTokenize=true;
	skipTokenize=false;
}

bool AtbStemmer::on_match() {
	vocalizedSolution="";
	QList<int> splits;
	QStringList splitVoc;
	int stemSplitStartIndex,stemSplitEndIndex;
	Status stat=EQ;
	int index=0;
	int i;
	for (i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty()) {
			continue;
		}
		splits.append(Prefix->sub_positionsOFCurrentMatch[i]);
		splitVoc.append(pre.POS.split("/")[0]);
		vocalizedSolution+=pre.raw_data;
		stat=equal(index,pre,stat);
		if (stat==D_ALL)
			return true;
	}
	assert(Prefix->sub_positionsOFCurrentMatch[i-1]+1==Stem->info.start);

	minimal_item_info & stem = *stem_info;
	stemSplitStartIndex=splits.size();
	stemSplitEndIndex=stemSplitStartIndex+stem.POS.count("+");
	for (int i=stemSplitStartIndex;i<=stemSplitEndIndex;i++) { //just to keep number of tokens aligned
		splits.append(Stem->info.finish);
		splitVoc.append(stem.POS.split("/")[0]);
		vocalizedSolution+=stem.raw_data;
	}
	stat=equal(index,stem,stat,ignoreStem);
	if (stat==D_ALL)
		return true;

	for (i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty()) {
			continue;
		}
		splits.append(Suffix->sub_positionsOFCurrentMatch[i]);
		splitVoc.append(suff.POS.split("/")[0]);
		vocalizedSolution+=suff.raw_data;
		stat=equal(index,suff,stat);
		if (stat==D_ALL)
			return true;
	}
	assert(Suffix->sub_positionsOFCurrentMatch[i-1]==Suffix->info.finish);
	if (index==voc.size()) {
		if (stat==EQ) {
			found=true;
			this->stat=EQ;
			stemPos=stem.POS;
		#ifdef TOKENIZE
			//tokenize
			int j=0;
			int o=info.start;

			for (int i=0;i<pos_after.size();i++) {
				int count=pos_after[i].count("+");
				j+=count;
				if (stemSplitStartIndex<=j && j<stemSplitEndIndex) {
					skipTokenize=true;
					return false;
				}
				int p=splits[j];
			#ifdef SPECIAL_TOKENIZE
				if (j+1 < splitVoc.size() && splitVoc[j+1].size()>0 && splitVoc[j].size()>0) {
					bool eq=false;
					QChar l1=splitVoc[j][splitVoc[j].size()-1];
					QChar l2=splitVoc[j+1][0];
					if (l1=='~') {
						if (splitVoc[j].size()>1) {
							QChar l0=splitVoc[j][splitVoc[j].size()-2];
							if (l0==l2) {
								eq=true;
							}
						}
					}
					if (l1==l2)
						eq=true;
					if (eq /*&& !splitVoc[j+1].startsWith("ya")*/)
						p=getLastLetter_index(*info.text,p-1);
				}
			#endif
				QString inp=info.text->mid(o,p-o+1);
				if (inp!=input_after[i])
					correctTokenize=false;
				sarfTokenization.append(inp);
				j++;
				o=p+1;
			}
		#endif
			return false;
		}
	} else {
		stat=D_ALL;
	}
	if (this->stat==D_VOC && stat==D_GL) //voc is worser
		this->stat=D_GL;
	else if (this->stat==D_ALL)
		this->stat=stat;
	return true;
}

inline QString removeDiacriticPOS(QString pos) {
	QStringList list=pos.split("+");
	QString l;
	pos="";
	foreach(l,list) {
		QString voc=l.split("/")[0];
		if (l.contains("CASE") || voc=="i" || voc=="u" || voc=="o" || voc=="a" || voc=="F" || voc=="K" || voc=="N")
			continue;
		pos+=l+"+";
	}
	if (pos[pos.size()-1]=='+')
		pos=pos.mid(0,pos.size()-1);
	return pos;
}

bool AtbStemmerContextFree::on_match() {
	num_solutions++;
#ifndef ATB_DIACRITICS
	QString pos,desc;
	for (int i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
			continue;
		desc+=pre.description()+" + ";
		pos+=pre.POS+"+";
	}
	minimal_item_info & stem = *stem_info;
	desc+=stem.description()+" + ";
	pos+=stem.POS+"+";

	for (int i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
			continue;
		desc+=suff.description()+" + ";
		pos+=suff.POS+"+";
	}
	(*f_out)<<desc<<"\t"<<pos<<"\n";
	QString modifiedPOS=removeDiacriticPOS(pos);
	modifiedPOSList.insert(modifiedPOS);
#endif
	return true;
}
#if 0
int getNumberPrefixes(QString pos) {
	static const QString pre[50]={"Aa/INTERROG_PART", ">a/IV1S", "Al/DET", "bi/PART", "bi/PREP", "fa/CONJ", "fa/CONNEC_PART",
							"fa/RC_PART", "fa/SUB_CONJ", "fiy/PREP", "|/IV1S", "ka/PREP", "la/EMPHATIC_PART", "lA/NEG_PART",
							"la/PREP", "la/RC_PART", "li/JUS_PART", "li/PREP", "mA/NEG_PART", "na/IV1P", "nu/IV1P",
							"sa/FUT_PART", "ta/IV2D", "ta/IV2FP", "ta/IV2FS", "ta/IV2MP", "ta/IV2MS", "ta/IV3FD",
							"ta/IV3FS", "ta/PREP", "tu/IV2D", "tu/IV2FP", "tu/IV2FS", "tu/IV2MP", "tu/IV2MS", "tu/IV3FD",
							"tu/IV3FS", ">u/IV1S", "wa/CONJ", "wa/PART", "wa/PREP", "wa/SUB_CONJ", "ya/IV3FP", "ya/IV3MD",
							"ya/IV3MP", "ya/IV3MS", "yu/IV3FP", "yu/IV3MD", "yu/IV3MP", "yu/IV3MS"};
	pos=removeDiacriticPOS(pos);
	int count=0;
	for (int i=0;i<50;i++) {
		if (pos.contains(pre[i]))
			count++;
	}
	return count;
}

int getNumberSuffixes(QString pos) {
	static const QString suff[177]={"a/CASE_DEF_ACC", "a/CASE_DEF_GEN", "a/CASE_INDEF_ACC", "a/CASE_INDEF_GEN",
									"AF/CASE_INDEF_ACC", "a/IVSUFF_MOOD:J", "a/IVSUFF_MOOD:S", "A/IVSUFF_SUBJ:3D",
									"A/IVSUFF_SUBJ:D_MOOD:SJ", "Ani/IVSUFF_SUBJ:3D_MOOD:I", "Ani/IVSUFF_SUBJ:D_MOOD:I",
									"Ani/NSUFF_MASC_DU_NOM", "A/NSUFF_MASC_DU_NOM", "A/NSUFF_MASC_DU_NOM_POSS",
									"ap/NSUFF_FEM_SG", "A/PVSUFF_SUBJ:3MD", "a/PVSUFF_SUBJ:3MS", "atAni/NSUFF_FEM_DU_NOM",
									"atA/NSUFF_FEM_DU_NOM_POSS", "atA/PVSUFF_SUBJ:3FD", "atayoni/NSUFF_FEM_DU_ACC",
									"atayoni/NSUFF_FEM_DU_GEN", "atayo/NSUFF_FEM_DU_ACC_POSS", "atayo/NSUFF_FEM_DU_GEN_POSS",
									"At/NSUFF_FEM_PL", "at/NSUFF_FEM_SG", "at/PVSUFF_SUBJ:3FS", "awoA/CVSUFF_SUBJ:2MP",
									"awoA/IVSUFF_SUBJ:MP_MOOD:SJ", "awoA/PVSUFF_SUBJ:3MP", "awo/CVSUFF_SUBJ:2MP",
									"awo/IVSUFF_SUBJ:3MP_MOOD:SJ", "awo/IVSUFF_SUBJ:MP_MOOD:SJ",
									"awona/IVSUFF_SUBJ:3MP_MOOD:I", "awona/IVSUFF_SUBJ:MP_MOOD:I", "aw/PVSUFF_SUBJ:3MP",
									"ayo/CVSUFF_SUBJ:2FS", "ayo/IVSUFF_SUBJ:2FS_MOOD:SJ", "ayona/IVSUFF_SUBJ:2FS_MOOD:I",
									"ayoni/NSUFF_MASC_DU_ACC", "ayoni/NSUFF_MASC_DU_GEN", "ayo/NSUFF_MASC_DU_ACC",
									"ayo/NSUFF_MASC_DU_ACC_POSS", "ayo/NSUFF_MASC_DU_GEN", "ayo/NSUFF_MASC_DU_GEN_POSS",
									"F/CASE_INDEF_ACC", "F/CASE_INDEF_GEN", "F/CASE_INDEF_NOM", "F/CASE_INDEF_NOM",
									"hA/CVSUFF_DO:3FS", "hA/IVSUFF_DO:3FS", "hA/POSS_PRON_3FS", "hA/PRON_3FS",
									"hA/PVSUFF_DO:3FS", "hu/CVSUFF_DO:3MS", "hu/IVSUFF_DO:3MS", "humA/CVSUFF_DO:3D",
									"humA/IVSUFF_DO:3D", "humA/POSS_PRON_3D", "humA/PRON_3D", "humA/PVSUFF_DO:3D",
									"hum/CVSUFF_DO:3MP", "hum/IVSUFF_DO:3MP", "hum/POSS_PRON_3MP", "hum/PRON_3MP",
									"hum/PVSUFF_DO:3MP", "hun~a/CVSUFF_DO:3FP", "hun~a/IVSUFF_DO:3FP", "hun~a/POSS_PRON_3FP",
									"hun~a/PRON_3FP", "hun~a/PVSUFF_DO:3FP", "hu/POSS_PRON_3MS", "hu/PRON_3MS",
									"hu/PVSUFF_DO:3MS", "i/CASE_DEF_ACC", "i/CASE_DEF_GEN", "iy/CVSUFF_SUBJ:2FS",
									"iy/IVSUFF_SUBJ:2FS_MOOD:SJ", "iyna/IVSUFF_SUBJ:2FS_MOOD:I", "iyna/NSUFF_MASC_PL_ACC",
									"iyna/NSUFF_MASC_PL_GEN", "iy/NSUFF_MASC_PL_ACC", "iy/NSUFF_MASC_PL_ACC_POSS",
									"iy/NSUFF_MASC_PL_GEN", "iy/NSUFF_MASC_PL_GEN_POSS", "iy/NSUFF_MASC_PL_NOM",
									"iy/POSS_PRON_1S", "~iy/PRON_1S", "iy/PRON_1S", "ka/IVSUFF_DO:2MS", "ka/POSS_PRON_2MS",
									"ka/PRON_2MS", "ka/PVSUFF_DO:2MS", "K/CASE_INDEF_ACC", "K/CASE_INDEF_GEN",
									"K/CASE_INDEF_NOM", "ki/IVSUFF_DO:2FS", "ki/POSS_PRON_2FS", "ki/PRON_2FS",
									"ki/PVSUFF_DO:2FS", "kumA/IVSUFF_DO:2D", "kumA/POSS_PRON_2D", "kumA/PRON_2D",
									"kumA/PVSUFF_DO:2D", "kum/IVSUFF_DO:2MP", "kum/POSS_PRON_2MP", "kum/PRON_2MP",
									"kum/PVSUFF_DO:2MP", "kun~a/IVSUFF_DO:2FP", "kun~a/POSS_PRON_2FP", "kun~a/PRON_2FP",
									"kun~a/PVSUFF_DO:2FP", "nA/CVSUFF_DO:1P", "nA/IVSUFF_DO:1P", "na/IVSUFF_SUBJ:3FP",
									"na/IVSUFF_SUBJ:FP", "nA/POSS_PRON_1P", "nA/PRON_1P", "nA/PVSUFF_DO:1P",
									"nA/PVSUFF_SUBJ:1P", "na/PVSUFF_SUBJ:3FP", "N/CASE_INDEF_NOM", "niy/CVSUFF_DO:1S",
									"niy/IVSUFF_DO:1S", "niy/PRON_1S", "niy/PVSUFF_DO:1S", "(null)/CVSUFF_SUBJ:2MS",
									"(null)/IVSUFF_MOOD:I", "(null)/IVSUFF_MOOD:J", "(null)/IVSUFF_MOOD:S",
									"(null)/PVSUFF_SUBJ:3MS", "o/CVSUFF_SUBJ:2MS", "o/IVSUFF_MOOD:J", "ta/PVSUFF_SUBJ:2MS",
									"ti/PVSUFF_SUBJ:2FS", "|t/NSUFF_FEM_PL", "tumA/PVSUFF_SUBJ:2D", "tum/PVSUFF_SUBJ:2MP",
									"tun~a/PVSUFF_SUBJ:2FP", "tu/PVSUFF_SUBJ:1S", "u/CASE_DEF_NOM", "u/CASE_INDEF_NOM",
									"u/IVSUFF_MOOD:I", "uwA/CVSUFF_SUBJ:2MP", "uwA/IVSUFF_SUBJ:MP_MOOD:SJ",
									"uwA/PVSUFF_SUBJ:3MP", "uw/CVSUFF_SUBJ:2MP", "uw/IVSUFF_SUBJ:3MP_MOOD:SJ",
									"uw/IVSUFF_SUBJ:MP_MOOD:SJ", "uwna/IVSUFF_SUBJ:3MP_MOOD:I", "uwna/IVSUFF_SUBJ:MP_MOOD:I",
									"uwna/NSUFF_MASC_PL_NOM", "uw/NSUFF_MASC_PL_NOM", "uw/NSUFF_MASC_PL_NOM_POSS",
									"uw/PVSUFF_SUBJ:3MP", "ya/POSS_PRON_1S", "ya/PRON_1S", "hi/CVSUFF_DO:3MS",
									"hi/IVSUFF_DO:3MS", "himA/CVSUFF_DO:3D", "himA/IVSUFF_DO:3D", "himA/POSS_PRON_3D",
									"himA/PRON_3D", "himA/PVSUFF_DO:3D", "him/CVSUFF_DO:3MP", "him/IVSUFF_DO:3MP",
									"him/POSS_PRON_3MP", "him/PRON_3MP", "him/PVSUFF_DO:3MP", "hin~a/CVSUFF_DO:3FP",
									"hin~a/IVSUFF_DO:3FP", "hin~a/POSS_PRON_3FP", "hin~a/PRON_3FP", "hin~a/PVSUFF_DO:3FP",
									"hi/POSS_PRON_3MS", "hi/PRON_3MS", "hi/PVSUFF_DO:3MS"};
	pos=removeDiacriticPOS(pos);
	int count=0;
	for (int i=0;i<177;i++) {
		if (pos.contains(suff[i]))
			count++;
	}
	return count;
}
#endif
inline QString readField(QStringList entries, QString field) {
	assert(entries.size()==2);
	assert(entries[0].contains(field));
	return entries[1];
}

inline QStringList readNextLineHelper(QTextStream & file, QString & line) {
	line=file.readLine(0);
	return line.split(": ",QString::KeepEmptyParts);
}


int atb(QString inputString, ATMProgressIFC * prg) {
#define readNextLine(file)  entries=readNextLineHelper(file, line)
#define readNextLineEntry(file,field) readField(readNextLineHelper(file, line),field)

	QFile conf("conflicts.txt");
	if (!conf.open(QIODevice::ReadWrite)) {
		out << "Conflicts File not found\n";
		return 1;
	}
	QTextStream conflicts(&conf);
#ifdef READ_CONFLICTS
	int conflict_index=-1;
	int conflictsSkipped=0;
#endif
	QFile morph_file("morph.txt");
	if (!morph_file.open(QIODevice::ReadWrite)) {
		out << "All Morphology Solutions File not found\n";
		return 1;
	}
	QTextStream morph(&morph_file);

	QDir folder(inputString+"/before","*.txt");
	if (!folder.exists()) {
		out << "Invalid Folder\n";
		return -1;
	}
	int found=0, notFound=0, notFoundGloss=0, notFoundVoc=0, appended=0, all_count=0,correctTokenize=0,skipTokenize=0,voc_conflict=0;
	QString file_name;
	int count=0;
	int num_files=folder.entryList().size();
	int ambiguity_sum[10]={0};
	int ambiguity_POS_sum[10]={0};
	int ambiguity_count[10]={0};
	int ambiguity_sum_detail[5][4]={0};
	int ambiguity_POS_sum_detail[5][4]={0};
	int ambiguity_count_detail[5][4]={0};

	const int max_diacritics=6;
	int diacritics[max_diacritics]={0};
	const int max_words_history=20;
	bool wrongDiacritics=false;
	int countWrong=0;
	QString oldInput;

	foreach (file_name,folder.entryList())	{
		QFile input(folder.absolutePath().append(QString("/").append(file_name)));
		QFile input_after(folder.absolutePath().append(QString("/").append(file_name)).replace("before","after"));
		if (!input.open(QIODevice::ReadOnly) || !input_after.open(QIODevice::ReadOnly)) {
			out << "File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		QTextStream file_after(&input_after);
		file_after.setCodec("utf-8");
		QStringList entries;
		QString line;
		while (!file.atEnd()) {
			all_count++;

			//read from before
			QString input_string=readNextLineEntry(file,"INPUT STRING");
			input_string=input_string.remove(kashida);
			readNextLine(file);
			readNextLine(file);
			readNextLine(file);
			readNextLine(file);
			int status =readNextLineEntry(file,"STATUS").toInt();
			readNextLine(file);
			QString unsplitvoc=readNextLineEntry(file,"UNSPLITVOC");
			QString pos =readNextLineEntry(file,"POS");
			QString voc =readNextLineEntry(file,"VOC");
			QString gloss =readNextLineEntry(file,"GLOSS");
			readNextLine(file);
			assert(line.isEmpty());
		#ifdef ATB_DIACRITICS
			if (wrongDiacritics) {
				if (countWrong>0) {
					oldInput+=" "+input_string;
					countWrong--;
				} else {
					error<<"Text: "<<oldInput<<"\n\n";
					wrongDiacritics=false;
					oldInput=getLastNWords(oldInput,max_words_history);
				}
			} else {
				if (all_count>max_words_history)
					removeFirstWord(oldInput);
				oldInput+=" "+input_string;
			}
			int c=countDiacritics(input_string);
			assert(unsplitvoc.size()>=2);
			if (unsplitvoc!="None"){
				unsplitvoc=unsplitvoc.mid(1,unsplitvoc.size()-2);
				unsplitvoc=Buckwalter::convertFrom(unsplitvoc);
			}
		#ifdef ATB_PRINT_ALL
			if (c>0) {
		#else
			if (status==1) {
		#endif
				if(unsplitvoc!="None" && !equal(input_string,unsplitvoc,true)) {
					wrongDiacritics=true;
					countWrong=max_words_history;
					error<<unsplitvoc<<"\t"<<input_string<<"\t"<<status<<"\n";
				}
				out<<unsplitvoc<<"\t"<<input_string<<"\t"<<c<<"\n";
				diacritics[c]++;
			}
			continue;
		#endif
			//read from after
			QStringList input_after, unvocalized_after, vocalized_after,pos_after;
		#ifdef TOKENIZE
			QString vocalized, vocalized_all;
			do {
				QString input_string=readNextLineEntry(file_after,"INPUT STRING");
				readNextLine(file_after);
				readNextLine(file_after);
				readNextLine(file_after);
				readNextLine(file_after);
				QString unvocalized=readNextLineEntry(file_after,"UNVOCALIZED");
				vocalized=readNextLineEntry(file_after,"VOCALIZED");
				QString pos=readNextLineEntry(file_after,"POS");
				readNextLine(file_after);
				readNextLine(file_after);
				assert(line.isEmpty());

				if (vocalized_after.size()>0) {
					assert(vocalized.startsWith("-"));
					vocalized_all+="+";
				}
				input_string.remove(kashida);
				input_after.append(input_string);
				unvocalized_after.append(unvocalized);
				QString voc_temp=vocalized;
				if (!input_string.contains("-")) //maybe it is just puncuation itself
					voc_temp.remove("-");
				vocalized_after.append(voc_temp);
				vocalized_all+=voc_temp;
				pos_after.append(pos);

			} while (vocalized.endsWith("-") && !input_string.contains("-"));
			assert(voc==vocalized_all);
		#endif
		#ifdef READ_CONFLICTS
			if (conflict_index==-1) {
				conflicts>>conflict_index;
			}
			if (conflict_index==all_count) {
				conflictsSkipped++;
				conflict_index=-1;
				continue;
			}
		#endif

			if (status==1 && !gloss.contains("NOT_IN_LEXICON") && input_string[input_string.size()-1]!=kashida) {
				QStringList p=pos.split("+");
				QStringList v=voc.split("+");
				QStringList g=gloss.split("+",QString::SkipEmptyParts);
				for (int i=0;i<g.size();i++) {
					if (g[i].startsWith(" "))
						g[i]=g[i].mid(1);
					if (g[i].endsWith(" "))
						g[i]=g[i].mid(0,g[i].size()-1);
				}
				AtbStemmer s(input_string,v,g,p,input_after,pos_after,true);
				s();
				AtbStemmer::Status stat=s.getStatus();
				if (s.isFound() || (stat ==AtbStemmer::D_GL && gloss.contains("nogloss"))) {
					found++;
					morph<<input_string<<"\n";
					AtbStemmerContextFree con(input_string,&morph);
					con();
					morph<<"\n";
					if (stat==AtbStemmer::EQ) {
						int amb=con.getAmbiguity();
						int amb_pos=con.getModifiedPosAmbiguity();
						int morpheme_count=p.size();
						ambiguity_sum[morpheme_count]+=amb;
						ambiguity_POS_sum[morpheme_count]+=amb_pos;
						ambiguity_count[morpheme_count]++;
						QString pos_mod=removeDiacriticPOS(pos);
						QString stem=s.getStemPOS();
						QString stemPos=stem.split("/")[1];
						QStringList pre_suff=pos_mod.split(stemPos);
						int num_pre=pre_suff[0].count("+");
						int num_suff=(pre_suff.size()>1?pre_suff[1].count("+"):0);
						ambiguity_sum_detail[num_pre][num_suff]+=amb;
						ambiguity_POS_sum_detail[num_pre][num_suff]+=amb_pos;
						ambiguity_count_detail[num_pre][num_suff]++;
					}
					if (s.isSkipTokenize()) {
						skipTokenize++;
					}else if (s.isCorrectlyTokenized()) {
						correctTokenize++;
					} else {
						out<<"Misaligned-Tokenization:\t";
						out	<<(found+notFound)<<") "<<file_name<<":\n"
							<<input_string<<"\t"<<voc<<"\t"<<pos<<"\t"<<gloss<<"\n";
						for (int i=0;i<input_after.size();i++) {
							if (i>0)
								out<<"+";
							out<<input_after[i];
						}
						out<<"\t";
						const QStringList & sarf=s.getTokenization();
						for (int i=0;i<sarf.size();i++) {
							if (i>0)
								out<<"+";
							out<<sarf[i];
						}
						out<<"\n";
					}
				#ifdef TOKENIZE
					if (Buckwalter::convertFrom(vocalized_all.remove("+"))!=s.getVocalizedSolution()) {
						voc_conflict++;
					}
				#endif
				} else {
					notFound++;

					if (stat!=AtbStemmer::D_ALL) {
						if (stat==AtbStemmer::D_GL) {
							notFoundGloss++;
							out <<"\tGLOSS:\n";
						} else if (stat==AtbStemmer::D_VOC) {
							notFoundVoc++;
							out <<"\tVOC:\n";
						}
						const QStringList & similar=s.getMostSimilar();
						for (int i=0;i<similar.size();i++)
							out<<"\t=>\t"<<similar.at(i)<<"\n";
					} else {
						out<<"\tNOT-Found:\n";
					}
					out	<<notFound<<"["<<notFoundGloss<<","<<notFoundVoc<<"]/"<<(found+notFound)<<") "<<file_name<<":\n"
						<<input_string<<"\t"<<voc<<"\t"<<pos<<"\t"<<gloss<<"\n";
					out<<"\n";

				#ifdef SAVE_CONFLICTS
					conflicts<<all_count<<"\n";
				#endif

				}
			#if 0
				out<<input_string;

				out<<"\t";
				assert(p.size()==v.size());
				for (int i=0;i<p.size();i++) {
					out<<v[i]<<"/"<<p[i];
					if (i!=p.size()-1)
						out<<"+";
				}
				out <<"\t";
				for (int i=0;i<input_after.size();i++) {
					out<<input_after[i];
					if (i!=input_after.size()-1)
						out<<"+";
				}

				out<<"\n";
			#endif
			}
			else if (status !=1) {
				if (alefs.contains(input_string[0]) && pos.startsWith("INTERROG_PART")) {
					appended++;
					out<<"\tAlef:\n";
					out	<<"("<<appended<<"/"<<all_count<<")"<<file_name<<":\n"
						<<input_string<<"\t"<<voc<<"\t"<<pos<<"\t"<<gloss<<"\n";
				}
			}
		}
		input.close();
		input_after.close();
		count++;
		prg->report((double)count/num_files*100+0.5);
	}

	conf.close();
	morph_file.close();

#ifndef ATB_DIACRITICS
	int total=found+notFound;
	int other=notFound-notFoundGloss-notFoundVoc;
	int tokenized=found-skipTokenize;
	double miss=((double)notFound)/total;
	double missGloss=((double)notFoundGloss)/total;
	double missVoc=((double)notFoundVoc)/total;
	double missOther=miss-missGloss-missVoc;
	double appendedRatio=((double)appended)/all_count;
	double correctTokenization=((double)correctTokenize)/tokenized;
	double skipTokenization=((double)skipTokenize)/found;
	double vocConflictRatio=((double)voc_conflict)/found;
	displayed_error	<<"Gloss Missing Rate=\t\t"<<notFoundGloss<<"/"<<total<<" =\t"<<missGloss<<"\n"
					<<"Vocalization Missing Rate=\t"<<notFoundVoc<<"/"<<total<<" =\t"<<missVoc<<"\n"
					<<"Other Missing Rate=\t\t"<<other<<"/"<<total<<" =\t"<<missOther<<"\n"
					<<"Total Missing Rate=\t\t"<<notFound<<"/"<<total<<" =\t"<<miss<<"\n"
					<<"Augmented Ratio=\t\t"<<appended<<"/"<<all_count<<"=\t"<<appendedRatio<<"\n"
					<<"Correct Tokenization Ratio=\t"<<correctTokenize<<"/"<<tokenized<<"=\t"<<correctTokenization<<"\n"
					<<"Skipped Tokenization Ratio=\t"<<skipTokenize<<"/"<<found<<"=\t"<<skipTokenization<<"\n"
					<<"Vocalization Conflict Ratio=\t"<<voc_conflict<<"/"<<found<<"=\t"<<vocConflictRatio<<"\n";
	displayed_error<<"\n";
	for (int i=0;i<10;i++) {
		double a=((double)ambiguity_sum[i])/ambiguity_count[i];
		displayed_error<<"AMB\t"<<i<<"\t"<<a<<"\n";
	}
	displayed_error<<"\n";
	for (int i=0;i<10;i++) {
		double a=((double)ambiguity_POS_sum[i])/ambiguity_count[i];
		displayed_error<<"AMB POS\t"<<i<<"\t"<<a<<"\n";
	}
	displayed_error<<"\n";
	for (int i=0;i<5;i++) {
		for (int j=0;j<4;j++) {
			double a=((double)(ambiguity_sum_detail[i][j]))/ambiguity_count_detail[i][j];
			displayed_error<<"AMB\t"<<i<<","<<j<<"\t"<<a<<"\n";
		}
	}
	displayed_error<<"\n";
	for (int i=0;i<5;i++) {
		for (int j=0;j<4;j++) {
			double a=((double)(ambiguity_POS_sum_detail[i][j]))/ambiguity_count_detail[i][j];
			displayed_error<<"AMB POS\t"<<i<<","<<j<<"\t"<<a<<"\n";
		}
	}
#else
	int c=0;
	for (int i=1;i<max_diacritics;i++) {
		double a=((double)diacritics[i])/all_count;
		c+=diacritics[i];
		displayed_error<<"Diacritics\t"<<i<<"\t"<<diacritics[i]<<"\t"<<a<<"\n";
	}
	double a=((double)c)/all_count;
	displayed_error<<"Diacritics\t*\t"<<c<<"\t"<<a<<"\n";
#endif
	displayed_error<<"\n";

	return 0;
}

int atb2(QString, ATMProgressIFC *) {
	QFile morph_file("morph.txt");
	QFile morph_file1("morph1.txt");
	if (!morph_file.open(QIODevice::ReadWrite) || !morph_file1.open(QIODevice::ReadWrite)) {
		out << "All Morphology Solutions File not found\n";
		return 1;
	}
	QTextStream morph1(&morph_file);
	QTextStream morph2(&morph_file1);
	int all_count=0,conflict_pos_count=0,conflict_desc_count=0,conflict_count_fa=0,added_count_alef=0,removed_count_wafa=0,added_count_inter=0;
	double conflict_pos_fraction=0,conflict_desc_fraction=0,conflict_fraction_fa=0,added_fraction_alef=0,removed_fraction_wafa=0,added_fraction_inter=0;
	while (!morph1.atEnd() && !morph2.atEnd()) {
		all_count++;

		QStringList desc1, desc2, pos1, pos2;
		//QString word1,word2;
		QString line;
		//word1=morph1.readLine(0);
		do {
			line=morph1.readLine(0);
			if (line.isEmpty())
				break;
			QStringList entries= line.split("\t",QString::KeepEmptyParts);
			desc1.append(entries[0]);
			pos1.append(entries[1]);
		} while (!line.isEmpty());
		//word2=morph2.readLine(0);
		do {
			line=morph2.readLine(0);
			if (line.isEmpty())
				break;
			QStringList entries= line.split("\t",QString::KeepEmptyParts);
			desc2.append(entries[0]);
			pos2.append(entries[1]);
		} while (!line.isEmpty());
		assert(desc1.size()==pos1.size() && desc2.size()==pos2.size());
		int maximum=max(pos1.size(),pos2.size());
		int minimum=min(pos1.size(),pos2.size());
		QStringList & descMin=(minimum==pos1.size()?desc1:desc2)
				, &descMax=(minimum==pos1.size()?desc2:desc1)
				, &posMin=(minimum==pos1.size()?pos1:pos2)
				, &posMax=(minimum==pos1.size()?pos2:pos1);
		if (minimum!=maximum) {
			bool alef=false;
			int diff_fa=0,diff_wafa=0,diff_inter=0;
			for (int i=0;i<maximum;i++) {
				if (posMax[i].startsWith("fa/RC_PART"))
					diff_fa++;
				if (posMax[i].startsWith(">a/IV1S") || posMax[i].startsWith(">u/IV1S") || posMax[i].startsWith("|/IV1S"))
					alef=true;
				if (posMax[i].startsWith("wa/CONJ+fa/CONJ"))
					diff_wafa++;
				if (posMax[i].startsWith("Aa/INTERROG_PART"))
					diff_inter++;
			}
			if (diff_fa>0)
				conflict_count_fa++;
			conflict_fraction_fa+=((double)diff_fa)/maximum;
			if (diff_wafa>0)
				removed_count_wafa++;
			removed_fraction_wafa+=((double)diff_wafa)/maximum;
			if (diff_inter>0)
				added_count_inter++;
			added_fraction_inter+=((double)diff_inter)/maximum;
			if (alef) {
				added_count_alef++;
				added_fraction_alef+=((double)(maximum-minimum))/maximum;
			}
		}

		int pos_diff=0, desc_diff=0;
		for (int i=0;i<descMin.size();i++) {
			if (!posMax.contains(posMin[i])) {
				pos_diff++;
				out<<"POS:\t"<<posMin[i]<<"\n";
			} else if (!descMax.contains(descMin[i])) {
				desc_diff++;
				out<<"GLOSS:\t"<<descMin[i]<<"\n";
			}
		}
		if (pos_diff>0)
			conflict_pos_count++;
		if (desc_diff>0)
			conflict_desc_count++;
		if (minimum>0) {
			conflict_pos_fraction+=((double)pos_diff)/minimum;
			conflict_desc_fraction+=((double)desc_diff)/minimum;
		}
	}
	morph_file.close();
	morph_file1.close();

	double pos_count_ratio=((double)conflict_pos_count)/all_count;
	double desc_count_ratio=((double)conflict_desc_count)/all_count;
	double pos_fraction_ratio=conflict_pos_fraction/all_count;
	double desc_fraction_ratio=conflict_desc_fraction/all_count;
	double conflict_count_ratio_fa=((double)conflict_count_fa)/all_count;
	double conflict_fraction_ratio_fa=conflict_fraction_fa/all_count;
	double removed_count_ratio_wafa=((double)removed_count_wafa)/all_count;
	double removed_fraction_ratio_wafa=removed_fraction_wafa/all_count;
	double added_count_ratio_alef=((double)added_count_alef)/all_count;
	double added_fraction_ratio_alef=added_fraction_alef/all_count;
	double added_count_ratio_inter=((double)added_count_inter)/all_count;
	double added_fraction_ratio_inter=added_fraction_inter/all_count;
	displayed_error	<<"POS Conflict Ratio=\t\t"<<conflict_pos_count<<"/"<<all_count<<" =\t"<<pos_count_ratio<<"\n"
					<<"Desc Conflict Ratio=\t\t"<<conflict_desc_count<<"/"<<all_count<<" =\t"<<desc_count_ratio<<"\n"
					<<"Fa Conflict Ratio=\t\t"<<conflict_count_fa<<"/"<<all_count<<" =\t"<<conflict_count_ratio_fa<<"\n"
					<<"WaFa Removed Ratio=\t\t"<<removed_count_wafa<<"/"<<all_count<<" =\t"<<removed_count_ratio_wafa<<"\n"
					<<"Alef Added Ratio=\t\t"<<removed_fraction_wafa<<"/"<<all_count<<" =\t"<<added_count_ratio_alef<<"\n"
					<<"Interrogation Added Ratio=\t"<<added_count_inter<<"/"<<all_count<<" =\t"<<added_count_ratio_inter<<"\n"
					<<"POS Conflict Fraction Ratio=\t"<<conflict_pos_fraction<<"/"<<all_count<<" =\t"<<pos_fraction_ratio<<"\n"
					<<"Desc Conflict Fraction Ratio=\t"<<conflict_desc_fraction<<"/"<<all_count<<" =\t"<<desc_fraction_ratio<<"\n"
					<<"Fa Conflict Fraction Ratio=\t"<<conflict_fraction_fa<<"/"<<all_count<<" =\t"<<conflict_fraction_ratio_fa<<"\n"
					<<"WaFa Removed Fraction Ratio=\t"<<removed_fraction_wafa<<"/"<<all_count<<" =\t"<<removed_fraction_ratio_wafa<<"\n"
					<<"Alef Added Fraction Ratio=\t"<<added_fraction_alef<<"/"<<all_count<<" =\t"<<added_fraction_ratio_alef<<"\n"
					<<"Interrogation Added Fraction Ratio=\t"<<added_fraction_inter<<"/"<<all_count<<" =\t"<<added_fraction_ratio_inter<<"\n";
	return 0;
}

