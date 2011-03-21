#include "logger.h"
#include "Search_by_item_locally.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stemmer.h"

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
		qDebug()<<"s:"<<info.text->mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
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
#endif
	return controller->on_match_helper();
}

bool SuffixMachine::shouldcall_onmatch(int position)
{
	if (position>=info.text->length() )
		return true;
	QChar ch=info.text->at(position);
	if (isDelimiter(ch))
		return true;
#ifdef RUNON_WORDS
	if (position>0)
	{
		ch=info.text->at(position-1);
		if (isNonConnectingLetter(ch))
		#ifdef REMOVE_ONE_LETTER_ABBREVIATIONS_FROM_BEING_IN_RUNONWORDS
			if (position-controller->Prefix->info.start>1)
		#endif
			{
				controller->machines.append(SubMachines(controller->Prefix,controller->Stem,controller->Suffix));
				return true;
			}
	}
#endif
	return false;
}

bool Stemmer::on_match_helper()
{
	info.finish=Suffix->info.finish;
#ifdef RUNON_WORDS
	machines.append(SubMachines(Prefix,Stem,Suffix));
	for (int i=0;i<machines.size();i++)
	{
		runwordIndex=i;
		SubMachines & m=machines[i];
		Prefix=m.Prefix;
		Stem=m.Stem;
		Suffix=m.Suffix;
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
	machines.removeLast();
	return true;
#endif
}
bool Stemmer::on_match()
{
	int count=0;
	int number=0;
	if (called_everything || type==PREFIX)
	{
		out<<QString(runwordIndex,'\t');
		out<<"(";
		for (int i=0;i<prefix_infos->count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item ( I dont think this is the case anymore)
		{
			if (number>0)
					out<<" + ";
			number++;
			if (count>0)
					out << " OR ";
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
			if (stem_info->abstract_categories[i])
				if (get_abstractCategory_id(i)>=0)
					out<<getColumn("category","name",get_abstractCategory_id(i))<< " ";
		out<<"]";
		out <<")-";
	}
	if (called_everything || type==SUFFIX)
	{
		out<< "-(";
		number=0;
		count=0;
		for (int i=0;i<suffix_infos->count();i++)
		{
			if (number>0)
					out<<" + ";
			number++;
			if (count>0)
					out << " OR ";
			count++;
			out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ suffix_infos->at(i).description();
		}
		out <<")";
	}
	if (called_everything)
	{
		QString word;
		for (int i=0;i<prefix_infos->count();i++)
			word.append(prefix_infos->at(i).raw_data);
		word.append(stem_info->raw_data);
		for (int i=0;i<suffix_infos->count();i++)
			word.append(suffix_infos->at(i).raw_data);
		out <<" "<<word<<" ";
	}
	out<<" "<<Prefix->info.start+1<<","<<Suffix->info.finish+1<<"\n";
	return true;
}
