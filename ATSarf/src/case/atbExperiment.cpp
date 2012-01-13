#include "atbExperiment.h"
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <assert.h>
#include "logger.h"
#include "stemmer.h"


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

	QDir folder(inputString+"/before","*.txt");
	if (!folder.exists()) {
		out << "Invalid Folder\n";
		return -1;
	}
	int found=0, notFound=0, notFoundGloss=0, notFoundVoc=0, appended=0, all_count=0,correctTokenize=0,skipTokenize=0;
	QString file_name;
	int count=0;
	int num_files=folder.entryList().size();
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
			readNextLine(file);
			QString pos =readNextLineEntry(file,"POS");
			QString voc =readNextLineEntry(file,"VOC");
			QString gloss =readNextLineEntry(file,"GLOSS");
			readNextLine(file);
			assert(line.isEmpty());

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
				QStringList v=voc.split("+");
				QStringList g=gloss.split("+",QString::SkipEmptyParts);
				QStringList p=pos.split("+");
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
					/*if (vocalized_all.remove("+")!=s.getVocalizedSolution()) {

					}*/
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
	displayed_error	<<"Gloss Missing Rate=\t\t"<<notFoundGloss<<"/"<<total<<" =\t"<<missGloss<<"\n"
					<<"Vocalization Missing Rate=\t"<<notFoundVoc<<"/"<<total<<" =\t"<<missVoc<<"\n"
					<<"Other Missing Rate=\t\t"<<other<<"/"<<total<<" =\t"<<missOther<<"\n"
					<<"Total Missing Rate=\t\t"<<notFound<<"/"<<total<<" =\t"<<miss<<"\n"
					<<"Augmented Ratio=\t\t"<<appended<<"/"<<all_count<<"=\t"<<appendedRatio<<"\n"
					<<"Correct Tokenization Ratio=\t"<<correctTokenize<<"/"<<tokenized<<"=\t"<<correctTokenization<<"\n"
					<<"Skipped Tokenization Ratio=\t"<<skipTokenize<<"/"<<found<<"=\t"<<skipTokenization<<"\n";
	return 0;
}
