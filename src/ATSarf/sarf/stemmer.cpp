#include "logger/logger.h"
#include "../sql-interface/Search_by_item.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "stemmer.h"

bool Stemmer::on_match_helper() //needed just to count matches till now
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
				Search_by_item s(PREFIX,Prefix->idsOFCurrentMatch[i]);
				minimal_item_info prefix_info;
				while(s.retrieve(prefix_info))//TODO: results with incorrect behaviour assuming more than 1 category works for any affix, but assuming just one match this works
				{
					#ifdef REDUCE_THRU_DIACRITICS
						if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] && prefix_info.raw_data==Prefix->raw_datasOFCurrentMatch[i])
					#else
						if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] )
					#endif
							prefix_infos->append(prefix_info);
				}
			}
			if (!called_everything)
				finish=Prefix->sub_positionsOFCurrentMatch.last();
		}
		if (called_everything || type==SUFFIX)
		{
			suffix_infos=new QVector<minimal_item_info>();
			#if !defined (REDUCE_THRU_DIACRITICS)
				Suffix->fill_details();
			#endif
			for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
			{
				Search_by_item s(SUFFIX,Suffix->idsOFCurrentMatch[i]);
				minimal_item_info suffix_info;
				while(s.retrieve(suffix_info))
				{
				#ifdef REDUCE_THRU_DIACRITICS
					if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i] && suffix_info.raw_data==Suffix->raw_datasOFCurrentMatch[i])
				#else
					if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i])
				#endif
						suffix_infos->append(suffix_info);
				}
			}
			finish=Suffix->sub_positionsOFCurrentMatch.last();
		}
		if (called_everything || type==STEM)
		{
			Search_by_item s(STEM,Stem->id_of_currentmatch);
			if (!called_everything)
				finish=Stem->currentMatchPos;
			stem_info=new minimal_item_info;
			while(s.retrieve(*stem_info))
			{
				#ifdef REDUCE_THRU_DIACRITICS
					if (stem_info->category_id==Stem->category_of_currentmatch && stem_info->raw_data==Stem->raw_data_of_currentmatch)
				#else
					if (stem_info->category_id==Stem->category_of_currentmatch)
				#endif
					if (!on_match())
						return false;
			}
			return true;
		}
		else
			return on_match();
	}
	else
	{
		return on_match();
	}
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
			out<</*Prefix->sub_positionsOFCurrentMatch[i]<<" "<<*/ prefix_infos->at(i).description;
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
		out<</*Stem->startingPos-1<<" "<<*/ stem_info->description;
		out<<" [ ";
		for (unsigned int i=0;i<stem_info->abstract_categories.size();i++)
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
			out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ suffix_infos->at(i).description;
		}
		out <<")";
	}
	out<<" "<<finish+1<<"\n";
	return true;
}
Stemmer::Stemmer(QString *text,int start_pos, bool get_info)
{
	prefix_infos=NULL;
	stem_info=NULL;
	suffix_infos=NULL;
	start=start_pos;
	finish=start_pos;
	this->diacritic_text=text;
	this->get_all_details=get_info;
	//this->text=removeDiacritics(text);
	Prefix=new PrefixSearch(this,start_pos);
	Stem=NULL;
	Suffix=NULL;
}
bool Stemmer::operator()()//if returns true means there was a match
{
        called_everything=true;
		//total_matches_till_now=0;
		return Prefix->operator ()();
		//return (total_matches_till_now>0);
}
bool Stemmer::operator()(item_types type)//if returns true means there was a match
{
        called_everything=false;
        this->type=type;
		//total_matches_till_now=0;
        if (type==PREFIX)
				return Prefix->operator ()();
        else if (type==STEM)
				return Stem->operator ()();
        else if (type==SUFFIX)
				return Suffix->operator ()();
		else
			return false;
		//return (total_matches_till_now>0);
}
Stemmer::~Stemmer()
{
        if (Suffix)
                delete Suffix;
        if (Stem)
                delete Stem;
        if (Prefix)
                delete Prefix;
}


