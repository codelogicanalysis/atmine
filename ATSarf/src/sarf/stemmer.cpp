#include "logger.h"
#include "Search_by_item_locally.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stemmer.h"

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
	/*if (controller->Stem!=NULL)
		delete controller->Stem;//TODO: change this allocation, deallocation to clear*/
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
	if (controller->called_everything)
	{
	#ifdef DEBUG
		out<<"s:"<<info.text->mid(info.start,currentMatchPos-info.start+1)<<"\n";
	#endif
		if (controller->Suffix!=NULL)
			delete controller->Suffix;//TODO: change this allocation, deallocation to clear
		controller->Suffix= new SuffixMachine(controller,currentMatchPos+1,controller->Prefix->resulting_category_idOFCurrentMatch,controller->Stem->category_of_currentmatch);
		controller->Suffix->setSolutionSettings(controller->multi_p);
		return (*controller->Suffix)();
	}
	else
	{
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
			controller->machines.removeLast();
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

bool Stemmer::on_match_helper()
{
	info.finish=Suffix->info.finish;
#ifdef RUNON_WORDS
	if (sarfParameters.enableRunonwords)
		machines.append(SubMachines(Prefix,Stem,Suffix));
	for (int i=0;i<machines.size();i++)
	{
		runwordIndex=i;
		if (sarfParameters.enableRunonwords)
		{
			SubMachines & m=machines[i];
			Prefix=m.Prefix;
			Stem=m.Stem;
			Suffix=m.Suffix;
		}
#endif
		if (get_all_details)
		{
			solution_position * s_inf=Suffix->computeFirstSolution();
			do
			{
				solution_position * p_inf=Prefix->computeFirstSolution();
				do
				{
					solution_position * S_inf=Stem->computeFirstSolution();
					do
					{
						suffix_infos=&Suffix->affix_info;
						prefix_infos=&Prefix->affix_info;
						stem_info=Stem->solution;
						if (!on_match())
							return false;
					}while (Stem->computeNextSolution(S_inf));
					delete S_inf;
				}while (Prefix->computeNextSolution(p_inf));
				delete p_inf;
			}while(Suffix->computeNextSolution(s_inf));
			delete s_inf;
		}
		else
			if (!on_match())
				return false;
#ifdef RUNON_WORDS
	}
	if (sarfParameters.enableRunonwords)
		machines.removeLast();
	return true;
#endif
}
bool Stemmer::on_match()
{
	int count=0;
	if (called_everything || type==PREFIX)
	{
		out<<QString(runwordIndex,'\t');
		out<<"(";
		for (int i=0;i<prefix_infos->count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item ( I dont think this is the case anymore)
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
			if (count>0)
				out << " ";//" + ";
			else {// (count==0)
				if (suffix_infos->size()>1 && desc[0]=='[' && desc.size()>0 && desc[desc.size()-1]==']') {
					desc="";
				}
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
	return true;
}
