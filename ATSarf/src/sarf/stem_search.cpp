#include "Search_by_item.h"
#include "text_handling.h"
#include "diacritics.h"
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

#ifdef USE_TRIE_WALK
bool StemSearch::check_for_terminal(int letter_index,ATTrie::Position pos)
{
	const StemNode * node=NULL;
	Search_StemNode s1;
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
			return true;
	}
	else
		return true;
	if (!on_match_helper(letter_index,s1))
	{
		stop=true;
		return false;
	}
	return true;
}
#endif

void StemSearch::traverse(int letter_index,ATTrie::Position pos)
{
	int length=info->diacritic_text->length();
	for (int i=letter_index;i<length && !stop;i++)
	{
#ifdef USE_TRIE
#ifdef USE_TRIE_WALK
		QChar current_letter=info->diacritic_text->at(i);
		//qDebug()<<"s:"<<current_letter;
		if (isDiacritic(current_letter))
			continue;
		else if (current_letter!=alef)//(!alefs.contains(current_letter))
		{
			if (!trie->walk(pos, current_letter))
				break;
			if (!check_for_terminal(i,pos))
				break;
		}
		else
		{
			for (int j=0;j<alefs.size();j++)
			{
				if (!stop && trie->isWalkable(pos,alefs[j]))
				{
					ATTrie::Position pos2=trie->clonePosition(pos);
					trie->walk(pos2, alefs[j]);
					if (!check_for_terminal(i,pos2))
						break;
					traverse(i+1,pos2);
					trie->freePosition(pos2);
				}
			}
			break;
			/*if (!trie->walk(pos, alef))
				break;*/
		}
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
#ifndef USE_TRIE_WALK
		if (!on_match_helper(i,s1))
			false_returned=true;
#endif
#ifndef USE_TRIE
		}
#endif
	}
}

bool StemSearch::operator()()
{
	ATTrie::Position pos = trie->startWalk();
	stop=false;
	traverse(starting_pos,pos);
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
#endif
	trie->freePosition(pos);
	return !stop;
}


bool StemSearch::on_match_helper(int last_letter_index,Search_StemNode s1)
{
#ifdef REDUCE_THRU_DIACRITICS
	minimal_item_info inf;
	while(s1.retrieve(inf))
	{
		//qDebug() << inf.raw_data;
		/*if (!false_returned)
		{*/
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
				QString subword=addlastDiacritics(starting_pos,last_letter_index,info->diacritic_text,last);
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
							/*false_returned=true;
							break;*/
							return false;
						}
					}
					else
					{
						if (!info->on_match_helper())
						{
							/*false_returned=true;
							break;*/
							return false;
						}
					}
				}
			}
		//}
	}
#else
	long cat_id;
	while(s1.retrieve(cat_id))
	{
		/*if (!false_returned)
		{*/
			if (database_info.rules_AB->operator ()(info->Prefix->resulting_category_idOFCurrentMatch,cat_id))
			{
				category_of_currentmatch=cat_id;
				currentMatchPos=i-1;
				if (info->called_everything)
				{
					if (!onMatch())
						//false_returned=true;
						return false;
				}
				else
				{
					if (!info->on_match_helper())
						//false_returned=true;
						return false;
				}
			}
		//}
	}
#endif
	return true;
}
bool StemSearch::onMatch()
{
#ifdef DEBUG
	out<<"s:"<<info->diacritic_text->mid(starting_pos,currentMatchPos-starting_pos+1)<<"-"<<raw_data_of_currentmatch<<"\n";
#endif
	info->Stem=this;
	SuffixSearch * Suffix= new SuffixSearch(info,currentMatchPos+1);
	return Suffix->operator ()();
}
