#include "logger.h"
#include "Search_by_item_locally.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stemmer.h"

void TreeMachine::get_all_possibilities(int i, QList< QString>  &raw_datas)
{
	if (i>possible_raw_datasOFCurrentMatch.count() || i<0)
		return;
	else if (i==possible_raw_datasOFCurrentMatch.count())
	{
		if (!a_branch_returned_false)
		{
			raw_datasOFCurrentMatch=raw_datas;
			if (controller->called_everything)
			{
				if (!postMatch())
					a_branch_returned_false=true;
			}
			else
			{
				if (!controller->on_match_helper())
					a_branch_returned_false=true;
			}
		}
	}
	else
	{
		for (int j=0;j<possible_raw_datasOFCurrentMatch[i].count();j++)
		{
			if (!a_branch_returned_false)
			{
				QList< QString> raw_datas_modified=raw_datas;
				raw_datas_modified.append(possible_raw_datasOFCurrentMatch[i][j]);
				get_all_possibilities(i+1,raw_datas_modified);
			}
			else
				break;
		}
	}
}

bool TreeMachine::onMatch()
{
#ifdef REDUCE_THRU_DIACRITICS
	QList<QString> raw_datas;
	raw_datas.clear();
	a_branch_returned_false=false;
	get_all_possibilities(0, raw_datas);
	return !a_branch_returned_false;
#else
	if (controller->called_everything)
		return postMatch();
	else
		return controller->on_match_helper();
	return true;
#endif
}

TreeMachine::TreeMachine(item_types type,Stemmer * controller,int start):TreeSearch(type,controller->info.text,start)
{
	this->controller =controller;
}

bool PrefixMachine::postMatch()
{
	//out<<"p:"<<info->word.mid(0,position)<<"\n";
	controller->Prefix=this;
	StemMachine *Stem=new StemMachine(controller,position);
	return Stem->operator ()();
}

StemMachine::StemMachine(Stemmer * controller,int start):StemSearch(controller->info.text,start)
{
	this->controller =controller;
}

bool StemMachine::shouldcall_onmatch(int)
{//check rules AB
	return (database_info.comp_rules->operator ()(controller->Prefix->getFinalResultingCategory(),category_of_currentmatch));
}

bool StemMachine::onMatch()
{
	if (controller->called_everything)
	{
	#ifdef DEBUG
		out<<"s:"<<info.text->mid(starting_pos,currentMatchPos-starting_pos+1)<<"-"<<raw_data_of_currentmatch<<"\n";
	#endif
		controller->Stem=this;
		SuffixMachine * Suffix= new SuffixMachine(controller,currentMatchPos+1);
		return Suffix->operator ()();
	}
	else
	{
		if (!controller->on_match_helper())
			return false;
	}
	return true;
}

bool SuffixMachine::postMatch()
{
	//out<<"S:"<<info->word.mid(startingPos)<<"\n";
	controller->Suffix=this;
	return controller->on_match_helper();
}

bool SuffixMachine::shouldcall_onmatch(int position)
{
	if (position>=info.text->length() || delimiters.contains(info.text->at(position)))
		if (database_info.comp_rules->operator ()(controller->Prefix->getFinalResultingCategory(),getFinalResultingCategory()) && database_info.comp_rules->operator ()(controller->Stem->category_of_currentmatch,getFinalResultingCategory()))
			return true;//check first is AC and second is BC
	return false;
}

bool Stemmer::on_match_helper()
{
	if (get_all_details)
	{

		if (called_everything || type==PREFIX)
		{
			prefix_infos=new QVector<minimal_item_info>();
			#if !defined (REDUCE_THRU_DIACRITICS)
				Prefix->fill_details();
			#endif
			for (int i=0;i<Prefix->sub_positionsOFCurrentMatch.count();i++)
			{
				Search_by_item_locally s(PREFIX,Prefix->idsOFCurrentMatch[i],Prefix->catsOFCurrentMatch[i],Prefix->raw_datasOFCurrentMatch[i]);
				minimal_item_info prefix_info;
				while(s.retrieve(prefix_info))//TODO: results with incorrect behaviour assuming more than 1 category works for any affix, but assuming just one match this works
					prefix_infos->append(prefix_info);
			}
			if (!called_everything)
				info.finish=Prefix->sub_positionsOFCurrentMatch.last();
		}
		if (called_everything || type==SUFFIX)
		{
			suffix_infos=new QVector<minimal_item_info>();
			#if !defined (REDUCE_THRU_DIACRITICS)
				Suffix->fill_details();
			#endif
			for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
			{
				Search_by_item_locally s(SUFFIX,Suffix->idsOFCurrentMatch[i],Suffix->catsOFCurrentMatch[i],Suffix->raw_datasOFCurrentMatch[i]);
				minimal_item_info suffix_info;
				while(s.retrieve(suffix_info))
					suffix_infos->append(suffix_info);
			}
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
