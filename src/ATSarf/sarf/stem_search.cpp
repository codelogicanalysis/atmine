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
	ATTrie::Position pos = trie->startWalk();
	int length=info->diacritic_text->length();
	for (int i=starting_pos;i<length && !false_returned;i++)
	{
#ifdef USE_TRIE
#ifdef USE_TRIE_WALK
		Search_StemNode s1;
		const StemNode * node=NULL;
		QChar current_letter=info->diacritic_text->at(i);
		//qDebug()<<"s:"<<current_letter;
		if (isDiacritic(current_letter))
			continue;
		else if (!alefs.contains(current_letter))
		{
			if (!trie->walk(pos, current_letter))
				break;
		}
		else
		{
			/*bool walked=false;
			for (int j=0;j<alefs.size();j++)
			{
				if (trie->isWalkable(pos,alefs[j])){
					trie->walk(pos, alefs[j]);
					walked=true;
					break;
				}
			}
			if (!walked)
				break;*/
			if (!trie->walk(pos, alef))
				break;
		}
		if (trie->isTerminal(pos))
		{
			ATTrie::Position pos2=trie->clonePosition(pos);
			trie->walk(pos2, '\0');
			node = trie->getData(pos2);
			trie->freePosition(pos2);
			if (node != NULL)
			{
				id_of_currentmatch=node->stem_id;
				s1.setNode(node);
			}
			else
				continue;
		}
		else
			continue;
#else
		QString name=info->diacritic_text.mid(starting_pos,i-starting_pos);
		const StemNode * node = NULL;
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
		Search_by_item s1(STEM,-1);
		bool not_finished=true;
		int j=0;
		QVector<QString> names;
		while (not_finished)
		{
			if (name.size()>0 && alefs.contains(name[0]))
			{
				if  (j<alefs.size())
				{
					QString t=alefs[j]+name.mid(1);
					Search_by_item s2(STEM,t);
					s1=s2;
					j++;
				}
				else
					not_finished=false;
				/*for (int j=0;j<alefs.size();j++)
					names.append(alefs[j]+name.mid(1));
				Search_by_item s2(STEM,name,names);
				s1=s2;
				not_finished=false;*/
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
#ifndef USE_TRIE_WALK
						currentMatchPos=i-1;
						QString subword=getDiacriticword(i-1,starting_pos,*info->diacritic_text);
#else
						//currentMatchPos=i;
						int last;
						QString subword=addlastDiacritics(starting_pos,i,info->diacritic_text,last);
								//info->diacritic_text->mid(starting_pos,i-starting_pos+1);
						currentMatchPos=last>0?last-1:0;
#endif
						//out<<"subword:"<<subword<<"-"<<raw_data_of_currentmatch<<currentMatchPos<<"\n";
						if (equal(subword,raw_data_of_currentmatch))
						{
							//out<<"yes\n";
							if (info->called_everything)
							{
								if (!onMatch())
								{
									false_returned=true;
									break;
								}
							}
							else
							{
								if (!info->on_match_helper())
								{
									false_returned=true;
									break;
								}
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
#ifdef USE_TRIE_WALK
	/*if (i == info->diacritic_text->length())
	{
		if (trie->isTerminal(pos))
		{
			trie->walk(pos, '\0');
			node = trie->getData(pos);
			if (node != NULL)
			{
				id_of_currentmatch=node->stem_id;
				s1.setNode(node);
			}
			else
				continue;
		}
		else
			continue;
	}
	else
		continue;*/
	trie->freePosition(pos);
#endif
	return !false_returned;
}
bool StemSearch::onMatch()
{
	//out<<"s:"<<info->diacritic_text->mid(starting_pos,currentMatchPos-starting_pos+1)<<"-"<<raw_data_of_currentmatch<<"\n";
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,currentMatchPos+1);
	return Suffix->operator ()();
}
