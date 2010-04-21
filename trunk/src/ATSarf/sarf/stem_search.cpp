#include "../sql-interface/Search_by_item.h"
#include "../utilities/text_handling.h"
#include "../utilities/diacritics.h"
#include "stem_search.h"
#include "stemmer.h"

StemSearch::StemSearch(Stemmer * info,int pos)
{
    this->info=info;
    starting_pos=pos;
#ifdef USE_TRIE
	trie=database_info.Stem_Trie;
#endif
}
StemSearch::~StemSearch(){	}
bool StemSearch::operator()()
{
	bool false_returned=false;
	for (int i=starting_pos;i<=info->word.length();i++)
	{
		QString name=info->word.mid(starting_pos,i-starting_pos);
#ifdef USE_TRIE

		QVector<QChar> alefs(4);
		alefs[0]=alef;
		alefs[1]=alef_hamza_above;
		alefs[2]=alef_hamza_below;
		alefs[3]=alef_madda_above;
#ifdef USE_TRIE_WALK
		Search_StemNode s1(NULL);
		StemNode * node=NULL;
		ATTrie::Position pos = trie->startWalk();
		int i = 0;
		for (i = 0; i < name.length(); i++)
		{
			if (!alefs.contains(name[i]))
			{
				if (!trie->walk(pos, name[i]))
					break;
			}
			else
			{
				bool walked=false;
				for (int j=0;j<alefs.size();j++)
				{
					if (trie->isWalkable(pos,alefs[j])){
						trie->walk(pos, alefs[j]);
						walked=true;
						break;
					}
				}
				if (!walked)
					break;
			}
		}
		if (i == name.length())
		{
			if (trie->isTerminal(pos))
			{
				node = trie->getData(pos);
				if (node != NULL)
				{
					id_of_currentmatch=node->stem_id;
					Search_StemNode s2(node);
					s1=s2;
				}
			}
			else
				continue;
		}
		else
			continue;
#else
		StemNode * node = NULL;
		trie->retreive(name,&node);
		if (node == NULL)
		{
			if (!alefs.contains(name[i]))
			{
				bool matched=false;
				for (int j=0;j<alefs.size();j++)
				{
					trie->retreive(alefs[j] + name.mid(1),&node);
					if (node != NULL){
						matched=true;
						break;
					}
				}
				if (!matched)
					continue;
			}
			else
				continue;
		}
		id_of_currentmatch=node->stem_id;
		Search_StemNode s1(node);
#endif
#else
		QVector<QChar> alefs(4);
		alefs[0]=alef;
		alefs[1]=alef_hamza_above;
		alefs[2]=alef_hamza_below;
		alefs[3]=alef_madda_above;
		Search_by_item s1(STEM,-1);
		bool not_finished=true;
		int j=0;
		QVector<QString> names;
		while (not_finished)
		{
			if (name.size()>0 && alefs.contains(name[0]))
			{
				/*if  (j<alefs.size())
				{
					QString t=;
					Search_by_item s2(STEM,t);
					s1=s2;
					j++;
				}
				else
					not_finished=false;*/
				for (int j=0;j<alefs.size();j++)
					names.append(alefs[j]+name.mid(1));
				Search_by_item s2(STEM,names);
				s1=s2;
				not_finished=false;
			}
			else
			{
				Search_by_item s2(STEM,name);
				s1=s2;
				not_finished=false;
			}
			id_of_currentmatch=s1.ID();
	#endif
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
#ifndef USE_TRIE
		}
#endif
	}
	return !false_returned;
}
bool StemSearch::onMatch()
{
	//out<<"s:"<<info->word.mid(starting_pos,currentMatchPos-starting_pos+1)<<"\n";
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,currentMatchPos+1);
	return Suffix->operator ()();
}
