#include "../logger/logger.h"
#include "prefix_search.h"
#include "stemmer.h"


PrefixSearch::PrefixSearch(Stemmer * info,int start):TreeSearch(PREFIX,info,start)
{
        //print_tree();
}
PrefixSearch::~PrefixSearch(){}
bool PrefixSearch::onMatch()
{
	//out<<"p:"<<info->word.mid(0,position)<<"\n";
	info->Prefix=this;
	StemSearch *Stem=new StemSearch(info,position);
	return Stem->operator ()();
}


