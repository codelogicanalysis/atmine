#include "logger.h"
#include "Search_by_item_locally.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stemmer.h"
#include "test.h"
#include "inflections.h"

SarfParameters sarfParameters;

PrefixMachine::PrefixMachine(Stemmer * controller,int start):PrefixSearch(controller->info.text,start)
{
	this->controller=controller;
}

bool PrefixMachine::onMatch()
{
#if 0
	for (node_info * part=lastNode();part!=NULL;part=previousNode(part))
		out<<"\n"<<info.text->mid(part->start,part->finish-part->start+1)<<"\n";
#endif
#ifdef DEBUG
	qDebug()<<"p:"<<info.text->mid(info.start,position)<<"\n";
#endif
	if (controller->Stem!=NULL)
		delete controller->Stem;//TODO: change this allocation, deallocation to clear
	controller->Stem=new StemMachine(controller,position,controller->Prefix->resulting_category_idOFCurrentMatch);
	controller->Stem->setSolutionSettings(controller->multi_p);
	return (*controller->Stem)();
}

StemMachine::StemMachine(Stemmer * controller,int start,long prefix_category):StemSearch(controller->info.text,start,prefix_category)
{
	this->controller =controller;
}

bool StemMachine::onMatch()
{
	if (controller->called_everything) {
	#ifdef DEBUG
		out<<"s:"<<info.text->mid(info.start,currentMatchPos-info.start+1)<<"\n";
	#endif
		if (controller->Suffix!=NULL)
			delete controller->Suffix;//TODO: change this allocation, deallocation to clear
		controller->Suffix= new SuffixMachine(controller,currentMatchPos+1,controller->Prefix->resulting_category_idOFCurrentMatch,controller->Stem->category_of_currentmatch);
		controller->Suffix->setSolutionSettings(controller->multi_p);
		return (*controller->Suffix)();
	} else {
		if (!controller->on_match_helper())
			return false;
	}
	return true;
}

SuffixMachine::SuffixMachine(Stemmer * controller,int start, long prefix_category,long stem_category):SuffixSearch(controller->info.text,start,prefix_category,stem_category)
{
	this->controller=controller;
}

bool SuffixMachine::onMatch()
{
#if 0
	for (node_info * part=lastNode();part!=NULL;part=previousNode(part))
		out<<"\n"<<info.text->mid(part->start,part->finish-part->start+1)<<"\n";
#endif
#ifdef DEBUG
	qDebug()<<"S:"<<info.text->mid(info.start,info.finish-info.start+1)<<"\n";
#endif
#ifdef RUNON_WORDS
	if (sarfParameters.enableRunonwords)
	{
		int index=controller->machines.size()-1;
		if (index>=0 && controller->machines[index].Suffix==controller->Suffix)
		{
			controller->Prefix=new PrefixMachine(controller,info.finish+1);
			controller->Prefix->setSolutionSettings(controller->multi_p);
			controller->Stem=NULL;
			controller->Suffix=NULL;
			bool result= (*controller->Prefix)();
			controller->Prefix=controller->machines[index].Prefix;
			controller->Stem=controller->machines[index].Stem;
			controller->Suffix=controller->machines[index].Suffix;
			//controller->machines.removeLast();
			controller->removeLastMachines();
			return result;
		}
	}
#endif
	return controller->on_match_helper();
}

bool SuffixMachine::shouldcall_onmatch(int position)
{
#if defined(RUNON_WORDS) && defined(REMOVE_ONE_LETTER_ABBREVIATIONS_FROM_BEING_IN_RUNONWORDS) //TODO:support for diacritics
	if (sarfParameters.enableRunonwords)
		if (position-controller->Prefix->info.start==1 && controller->machines.size()>0 && (*info.text)[position-1]!=waw)
			return false;
#endif
	if (position>=info.text->length() )
		return true;
	QChar ch=info.text->at(position);
	if (isDelimiter(ch))
		return true;
#ifdef RUNON_WORDS
	if (sarfParameters.enableRunonwords)
	{
		int lastLetterIndex=getLastLetter_index(*info.text,position-1);
		if (lastLetterIndex>=0)
		{
			ch=info.text->at(lastLetterIndex);
			if (isNonConnectingLetter(ch))
			#ifdef REMOVE_ONE_LETTER_ABBREVIATIONS_FROM_BEING_IN_RUNONWORDS
				if (position-controller->Prefix->info.start>1)
			#endif
				{
					controller->machines.append(SubMachines(controller->Prefix,controller->Stem,controller->Suffix));
					return true;
				}
		}
	}
#endif
	return false;
}

void Stemmer::removeLastMachines() {
	SubMachines last=machines.last();
	machines.removeLast();
	if (last.Prefix!=NULL)
		delete last.Prefix;
	if (last.Stem!=NULL)
		delete last.Stem;
	if (last.Suffix!=NULL)
		delete last.Suffix;
}

bool Stemmer::on_match_helper()
{
	info.finish=Suffix->info.finish;
#ifdef RUNON_WORDS
	if (sarfParameters.enableRunonwords)
		machines.append(SubMachines(Prefix,Stem,Suffix));
	for (int i=0;i<machines.size();i++) {
		runwordIndex=i;
		if (sarfParameters.enableRunonwords) {
			SubMachines & m=machines[i];
			Prefix=m.Prefix;
			Stem=m.Stem;
			Suffix=m.Suffix;
		}
#endif
		if (get_all_details) {
			solution_position * s_inf=Suffix->computeFirstSolution();
			do {
				solution_position * p_inf=Prefix->computeFirstSolution();
				do {
					solution_position * S_inf=Stem->computeFirstSolution();
					do {
						suffix_infos=&Suffix->affix_info;
						prefix_infos=&Prefix->affix_info;
						stem_info=Stem->solution;

						for (int j=0;j<2;j++) {
							QVector<minimal_item_info> * infos=(j==0?prefix_infos:suffix_infos);
							QList<result_node *> * results=(j==0?Prefix->result_nodes:Suffix->result_nodes);
							for (int i=0;i<results->size();i++) {
								QString inflectionRule=results->at(i)->getInflectionRule();
								minimal_item_info & current=(*infos)[i];
								if (i>1) {
									minimal_item_info & previous=(*infos)[i-1];
									applyInflections(inflectionRule,previous,current);
								} else {
									applyInflections(inflectionRule,current);
								}
							}
						}
						int count=0;
						if (suffix_infos->size()>0) { //maybe not best solution working for now
							minimal_item_info & info=(*suffix_infos)[0];
							QStringRef diacritics_raw_after =addlastDiacritics(-1,-1,&info.raw_data);
							if (diacritics_raw_after.size()>0) {
								QStringRef diacritics_before=getDiacriticsBeforePosition(Suffix->info.start,Suffix->info.text);
								count=0;
								int i=0;
								int s_r=diacritics_raw_after.size()-1;
								int s=diacritics_before.size()-1;
								int m=min(diacritics_raw_after.size(),diacritics_before.size());
								while (i<m && diacritics_raw_after.at(s_r-i)==diacritics_before.at(s-i)) { //either common ending
									count++;
									i++;
								}
								if (count==0) {
									int i=0;
									while (i<m && diacritics_raw_after.at(i)==diacritics_before.at(i)) { //or common starting
										count++;
										i++;
									}
								}
								Suffix->info.start-=count;
								Stem->info.finish-=count;
							}
						}

						if (!on_match())
							return false;
						if (count>0) {
							Suffix->info.start+=count;
							Stem->info.finish+=count;
						}
					}while (Stem->computeNextSolution(S_inf));
					delete S_inf;
				}while (Prefix->computeNextSolution(p_inf));
				delete p_inf;
			}while(Suffix->computeNextSolution(s_inf));
			delete s_inf;
		} else {
			if (!on_match())
				return false;
		}
#ifdef RUNON_WORDS
	}
	if (sarfParameters.enableRunonwords) {
		removeLastMachines();
	}
	return true;
#endif
}
bool Stemmer::on_match() {
#ifdef MORPHEME_TOKENIZE
	out	<<"ALTERNATIVE:\t";
	QString word;
	for (int i=0;i<prefix_infos->size();i++)
		word.append(prefix_infos->at(i).raw_data);
	word.append(stem_info->raw_data);
	for (int i=0;i<suffix_infos->size();i++)
		word.append(suffix_infos->at(i).raw_data);
	out <<" "<<word<<"\n";
	//out<<"\t("<<Prefix->info.start+1<<","<<Suffix->info.finish+1<<")\n";

	int pos=Prefix->info.start;
	for (int i=0;i<prefix_infos->size();i++) {
		minimal_item_info & pre = (*prefix_infos)[i];
		int pos2=Prefix->sub_positionsOFCurrentMatch[i];
		if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
			continue;
		out	<<"PREFIX:\t"
			<<info.text->mid(pos,pos2-pos+1)<<"\t"
			<<pre.raw_data<<"\t"
			<<"\""<<pre.description()<<"\"\t"
			<<pre.POS<<"\n";
		pos=pos2+1;
	}

	pos=Stem->info.start;
	minimal_item_info & stem = *stem_info;
	int pos2=Stem->info.finish;
	out	<<"STEM:\t"
		<<info.text->mid(pos,pos2-pos+1)<<"\t"
		<<stem.raw_data<<"\t"
		<<"\""<<stem.description()<<"\"\t"
		<<stem.POS<<"\n";

	pos=Suffix->info.start;
	for (int i=0;i<suffix_infos->size();i++) {
		minimal_item_info & suff = (*suffix_infos)[i];
		int pos2=Suffix->sub_positionsOFCurrentMatch[i];
		if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
			continue;
		out	<<"SUFFIX:\t"
			<<info.text->mid(pos,pos2-pos+1)<<"\t"
			<<suff.raw_data<<"\t"
			<<"\""<<suff.description()<<"\"\t"
			<<suff.POS<<"\n";
		pos=pos2+1;
	}
	assert(pos-1==Suffix->info.finish);
	out<<"\n";

#else
	int count=0;
	if (called_everything || type==PREFIX)
	{
		out<<QString(runwordIndex,'\t');
		out<<"(";
		for (int i=0;i<prefix_infos->count();i++)
		{
			if (count>0)
					out << " + ";
			count++;
			const minimal_item_info & rmii = prefix_infos->at(i);
			out<</*Prefix->sub_positionsOFCurrentMatch[i]<<" "<<*/ rmii.description();
		}
		out <<")-";
	}
	if (called_everything || type==STEM)
	{
		out<< "-(";
		count=0;
		if (count>0)
				out << " OR ";
		count++;
		out<</*Stem->startingPos-1<<" "<<*/ stem_info->description();
		out<<" [ ";
		for (unsigned int i=0;i<stem_info->abstract_categories.length();i++)
			if (stem_info->abstract_categories[i]) {
				int abstract_id=database_info.comp_rules->getAbstractCategoryID(i);
				if (abstract_id>=0)
					out<<database_info.comp_rules->getCategoryName(abstract_id)<< " ";
			}
		out<<"]";
		out <<")-";
	}
	if (called_everything || type==SUFFIX) {
		QString later_part="";
		out<< "-(";
		count=0;
		for (int i=0;i<suffix_infos->size();i++) {
			//qDebug()<< "{"<<suffix_infos->at(i).POS<<"}";
			QString desc=suffix_infos->at(i).description();
			if (count>0) {
			#ifdef SAMA
				out << " + ";
			#else
				out << " ";
			#endif
			} else {// (count==0)
			#ifdef SAMA
				if (suffix_infos->size()>1 && desc[0]=='[' && desc.size()>0 && desc[desc.size()-1]==']') {
					desc="";
				}
			#endif
			}
			count++;
			if (later_part=="" && suffix_infos->count()>i+1 && isReverseDirection(suffix_infos->at(i).abstract_categories)) {
				later_part=desc;
				//out<< "{"<<suffix_infos->at(i).POS<<"}";
				count =0;
				continue;
			} else {
				later_part="";
			}
			if (desc.isEmpty())
				count=0;
			out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ desc<<later_part;
			//out<< "{"<<suffix_infos->at(i).POS<<"}";
		}
		out <<")";
	}
	if (called_everything)
	{
		QString word;
		for (int i=0;i<prefix_infos->size();i++)
			word.append(prefix_infos->at(i).raw_data);
		word.append(stem_info->raw_data);
		for (int i=0;i<suffix_infos->size();i++)
			word.append(suffix_infos->at(i).raw_data);
		//QString suff;
		//for (int i=0;i<suffix_infos->count();i++)
		//	suff.append("-").append(suffix_infos->at(i).raw_data);
		out <<" "<<word;//<<" "<<"["<<suff<<"]";
	}
	out<<" "<<Prefix->info.start+1<<","<<Suffix->info.finish+1<<"\n";
#endif
	return true;
}
