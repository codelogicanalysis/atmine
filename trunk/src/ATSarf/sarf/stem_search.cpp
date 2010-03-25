#include "../sql-interface/Search_by_item.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "stem_search.h"
#include "stemmer.h"
#include "../common_structures/tree.h"

StemSearch::StemSearch(Stemmer * info,int pos):TreeSearch(STEM,info,pos)
{
    starting_pos=pos;
}
StemSearch::~StemSearch(){	}
/*bool StemSearch::operator()()
{
	bool false_returned=false;
	for (int i=starting_pos;i<=info->word.length();i++)
	{
		QString name=info->word.mid(starting_pos,i-starting_pos);
		//out<<name<<"\n";
		Search_by_item s1(STEM,name);
		id_of_currentmatch=s1.ID();
#ifdef REDUCE_THRU_DIACRITICS
		minimal_item_info inf;
		while(s1.retrieve(inf))
		{
			if (!false_returned)
			{
				if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,inf.category_id))
				{
					category_of_currentmatch=inf.category_id;
					raw_data_of_currentmatch=inf.raw_data;
					currentMatchPos=i-1;
					QString subword=getDiacriticword(i-1,starting_pos,info->diacritic_word);
					//out<<"subword:"<<subword<<"-"<<raw_data_of_currentmatch<<currentMatchPos<<"\n";
					if (equal(subword,raw_data_of_currentmatch))
					{
						//out<<"yes\n";
						if (info->called_everything)
						{
							if (!onMatch())
								false_returned=true;
						}
						else
						{
							if (!info->on_match_helper())
									false_returned=true;
						}
					}
				}
			}
		}
#else
		long cat_id;
		while(s1.retrieve(cat_id))
		{
			if (!false_returned)
			{
				if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,cat_id))
				{
					category_of_currentmatch=cat_id;
					currentMatchPos=i-1;
					if (info->called_everything)
					{
						if (!onMatch())
							false_returned=true;
					}
					else
					{
						if (!info->on_match_helper())
								false_returned=true;
					}
				}
			}
		}
#endif
	}
	return !false_returned;
}*/
bool StemSearch::onMatch()
{
#ifdef REDUCE_THRU_DIACRITICS
	raw_data_of_currentmatch=raw_datasOFCurrentMatch[0];
#endif
	//out<<"s:"<<info->word.mid(starting_pos,position-starting_pos)<<"\n";
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,position);
	return Suffix->operator ()();
}
bool StemSearch::shouldcall_onmatch(int)//re-implemented in case of SUFFIX search tree
{
	if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,resulting_category_idOFCurrentMatch))
		return true;
	return false;
}
void StemSearch::fill_details()
{
	TreeSearch::fill_details();
	currentMatchPos=position-1;
	category_of_currentmatch=catsOFCurrentMatch[0];
	id_of_currentmatch=idsOFCurrentMatch[0];
}
