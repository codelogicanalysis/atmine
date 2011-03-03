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
	//out<<"p:"<<info->word.mid(0,position)<<"\n";
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
	if (controller->called_everything)
	{
	#ifdef DEBUG
		out<<"s:"<<info.text->mid(starting_pos,currentMatchPos-starting_pos+1)<<"-"<<raw_data_of_currentmatch<<"\n";
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
	//out<<"S:"<<info->word.mid(startingPos)<<"\n";
	return controller->on_match_helper();
}

bool SuffixMachine::shouldcall_onmatch(int position)
{
	if (position>=info.text->length() )
		return true;
	QChar ch=info.text->at(position);
	if (isDelimiter(ch))
		return true;
	return false;
}

bool Stemmer::on_match_helper()
{
#if 0
	if (get_all_details)
	{
		if (called_everything || type==PREFIX)
		{
		#if !defined (REDUCE_THRU_DIACRITICS)
			Prefix->fill_details();
		#endif
		#ifndef MULTIPLICATION
			prefix_infos=new QVector<minimal_item_info>();

			for (int i=0;i<Prefix->sub_positionsOFCurrentMatch.count();i++)
			{
				Search_by_item_locally s(PREFIX,Prefix->idsOFCurrentMatch[i],Prefix->catsOFCurrentMatch[i],Prefix->raw_datasOFCurrentMatch[i]);
				minimal_item_info prefix_info;
				while(s.retrieve(prefix_info))//TODO: results with incorrect behaviour assuming more than 1 category works for any affix, but assuming just one match this works
					prefix_infos->append(prefix_info);
			}
		#else
			prefix_infos=Prefix->affix_info;
		#endif
			if (!called_everything)
				info.finish=Prefix->sub_positionsOFCurrentMatch.last();
		}
		if (called_everything || type==SUFFIX)
		{
		#if !defined (REDUCE_THRU_DIACRITICS)
			Suffix->fill_details();
		#endif
		#ifndef MULTIPLICATION
			suffix_infos=new QVector<minimal_item_info>();
			for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
			{
				Search_by_item_locally s(SUFFIX,Suffix->idsOFCurrentMatch[i],Suffix->catsOFCurrentMatch[i],Suffix->raw_datasOFCurrentMatch[i]);
				minimal_item_info suffix_info;
				while(s.retrieve(suffix_info))
					suffix_infos->append(suffix_info);
			}
		#else
			suffix_infos=Suffix->affix_info;
		#endif
			info.finish=Suffix->sub_positionsOFCurrentMatch.last();
		}
		if (called_everything || type==STEM)
		{
			Search_by_item_locally s(STEM,Stem->id_of_currentmatch,Stem->category_of_currentmatch,Stem->raw_data_of_currentmatch);
			if (!called_everything)
				info.finish=Stem->currentMatchPos;
			stem_info=new minimal_item_info;
			while(s.retrieve(*stem_info))
			{
				if (!on_match())
					return false;
			}
			return true;
		}
		else
			return on_match();
	}
	else
		return on_match();
#endif
	info.finish=Suffix->info.finish;
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
					suffix_infos=Suffix->affix_info;
					prefix_infos=Prefix->affix_info;
					stem_info=Stem->solution;
					if (!on_match())
						return false;
				}while (Stem->computeNextSolution(S_inf));
				delete S_inf;
			}while (Prefix->computeNextSolution(p_inf));
			delete p_inf;
		}while(Suffix->computeNextSolution(s_inf));
		delete s_inf;
		return true;
	}
	else
		return on_match();
}
bool Stemmer::on_match()
{
	int count=0;
	int number=0;
	if (called_everything || type==PREFIX)
	{
		out<<"(";
		for (int i=0;i<prefix_infos->count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item
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
	out<<" "<<info.finish+1<<"\n";
	return true;
}
