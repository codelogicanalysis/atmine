#include "suffix_search.h"
#include "stemmer.h"

SuffixSearch::SuffixSearch(Stemmer * info,int position):TreeSearch(SUFFIX,info,position)
{
    //print_tree();
}
SuffixSearch::~SuffixSearch()
{
}

bool SuffixSearch::shouldcall_onmatch(int position)//note that provided position is 1+last_letter
{
	if (position==info->word.length())
		if (database_info.rules_AC->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,resulting_category_idOFCurrentMatch) && database_info.rules_BC->operator ()(info->Stem->category_of_currentmatch,resulting_category_idOFCurrentMatch))
			return true;
	return false;
}
bool SuffixSearch::onMatch()
{
	//out<<"S:"<<info->word.mid(startingPos)<<"\n";
	info->Suffix=this;
	return info->on_match_helper();
}

