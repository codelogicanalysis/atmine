#include "logger/logger.h"
#include "../sql-interface/Search_by_item.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "stemmer.h"

bool Stemmer::on_match_helper() //needed just to count matches till now
{
		//total_matches_till_now++;
        return on_match();
}
bool Stemmer::on_match()
{
        int count=0;
        int number=0;
        if (called_everything || type==PREFIX)
        {
#if !defined (REDUCE_THRU_DIACRITICS)
                Prefix->fill_details();
#endif
                out<<"(";
                for (int i=0;i<Prefix->sub_positionsOFCurrentMatch.count();i++) //TODO: results with incorrect behaviour assuming more than 1 category works for any item
                {
                        //out<<Prefix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Prefix->catsOFCurrentMatch[i])<<" ";
                        if (number>0)
                                out<<" + ";
                        number++;
                        Search_by_item s(PREFIX,Prefix->idsOFCurrentMatch[i]);
                        minimal_item_info prefix_info;
                        while(s.retrieve(prefix_info))
                        {
#ifdef REDUCE_THRU_DIACRITICS
								if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] && prefix_info.raw_data==Prefix->raw_datasOFCurrentMatch[i])
#else
                                if (prefix_info.category_id==Prefix->catsOFCurrentMatch[i] )
#endif
                                {
                                        if (count>0)
                                                out << " OR ";
                                        count++;
                                        out<</*Prefix->sub_positionsOFCurrentMatch[i]<<" "<<*/ prefix_info.description;
                                }
                        }
                }
                out <<")-";
        }
        minimal_item_info stem_info;
        if (called_everything || type==STEM)
        {
                out<< "-(";
#if !defined (REDUCE_THRU_DIACRITICS)
				Stem->fill_details();
#endif
                //out<<Suffix->startingPos-1<<" "<<getColumn("category","name",Stem->category_of_currentmatch)<<" --- ";
                Search_by_item s(STEM,Stem->id_of_currentmatch);
                count=0;
                while(s.retrieve(stem_info))
                {
#ifdef REDUCE_THRU_DIACRITICS
						if (stem_info.category_id==Stem->category_of_currentmatch && stem_info.raw_data==Stem->raw_data_of_currentmatch)
#else
                        if (stem_info.category_id==Stem->category_of_currentmatch)
#endif
                        {
                                if (count>0)
                                        out << " OR ";
                                count++;
                                out<</*Stem->startingPos-1<<" "<<*/ stem_info.description;
                                out<<" [ ";
                                for (unsigned int i=0;i<stem_info.abstract_categories.size();i++)
									if (stem_info.abstract_categories[i])
										if (get_abstractCategory_id(i)>=0)
											out<<getColumn("category","name",get_abstractCategory_id(i))<< " ";
                                out<<"]";
                        }
                }
                out <<")-";
        }
        if (called_everything || type==SUFFIX)
        {
                out<< "-(";
#if !defined (REDUCE_THRU_DIACRITICS)
                Suffix->fill_details();
#endif
                number=0;
                count=0;
                for (int i=0;i<Suffix->sub_positionsOFCurrentMatch.count();i++)
                {
                        if (number>0)
                                out<<" + ";
                        number++;
                        //out<<Suffix->sub_positionsOFCurrentMatch[i]<<" "<< getColumn("category","name",Suffix->catsOFCurrentMatch[i])<<" ";
                        Search_by_item s(SUFFIX,Suffix->idsOFCurrentMatch[i]);
                        minimal_item_info suffix_info;
                        while(s.retrieve(suffix_info))
                        {
#ifdef REDUCE_THRU_DIACRITICS
								if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i] && suffix_info.raw_data==Suffix->raw_datasOFCurrentMatch[i])
#else
                                if (suffix_info.category_id==Suffix->catsOFCurrentMatch[i])
#endif
                                {
                                        if (count>0)
                                                out << " OR ";
                                        count++;
                                        out<</*Suffix->sub_positionsOFCurrentMatch[i]<<" "<<*/ suffix_info.description;
                                }
                        }
                }
                out <<")";
        }
        out<<"\n";
        return true;
}
Stemmer::Stemmer(QString word)
{
        this->diacritic_word=word;
        this->word=removeDiacritics(word);
        Prefix=new PrefixSearch(this);
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


