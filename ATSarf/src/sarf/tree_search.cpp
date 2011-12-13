#include "tree_search.h"
#include "logger.h"
#include "suffix_search.h"
#include "stem_search.h"
#include "prefix_search.h"
#include <QVector>
#include "node.h"
#include "tree.h"
#include "text_handling.h"
#include "diacritics.h"
#include "database_info_block.h"
#include <QDebug>
#include <assert.h>
#include <QString>

void TreeSearch::fill_details() //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
{
#ifdef QUEUE
	//nothing needs to be done, members are already filled during traversals
#elif defined(PARENT)
	if (!filled_details) {
        sub_positionsOFCurrentMatch.clear();
	#ifndef MULTIPLICATION
        catsOFCurrentMatch.clear();
        idsOFCurrentMatch.clear();
        catsOFCurrentMatch.insert(0,((result_node *)reached_node)->get_previous_category_id());
        idsOFCurrentMatch.insert(0, ((result_node *)reached_node)->get_affix_id());
	#endif
	#ifdef REDUCE_THRU_DIACRITICS
        possible_raw_datasOFCurrentMatch.clear();
		possible_raw_datasOFCurrentMatch.prepend(((result_node *)reached_node)->raw_datas);
		//qDebug()<<position<<":"<<((result_node *)reached_node)->raw_datas[0];
	#endif
		int index=getPositionOneLetterBackward(position,info.text);
		sub_positionsOFCurrentMatch.prepend(index);
        letter_node * tree_head=(letter_node*)Tree->getFirstNode();
        node * current_parent=((result_node *)reached_node)->parent;
		assert(current_parent->getResultChildren()->contains(reached_node));

		while (current_parent!=tree_head) {
			if (current_parent->isLetterNode()) {
				index=getLastLetter_index(*info.text,index);
				if (((letter_node* )current_parent)->getLetter()!='\0')
					index--;
			} else {
			#ifndef MULTIPLICATION
				catsOFCurrentMatch.insert(0,((result_node *)current_parent)->get_previous_category_id());
				idsOFCurrentMatch.insert(0, ((result_node *)current_parent)->get_affix_id());//was : reached_node
			#endif
				sub_positionsOFCurrentMatch.prepend(index);
			#ifdef REDUCE_THRU_DIACRITICS
				possible_raw_datasOFCurrentMatch.prepend(((result_node *)current_parent)->raw_datas);
				//qDebug()<<position-count<<":"<<((result_node *)current_parent)->raw_datas[0];
			#endif
			}
			current_parent=current_parent->parent;
        }
		//qDebug()<<"--";
		filled_details=true;
	}
#endif
}
bool TreeSearch::operator ()()
{
	QQueue<letter_node *> queue;
	queue.clear();
#ifdef QUEUE
	QList <long> catsOFCurrentMatch;
	QList<int> sub_positionsOFCurrentMatch;
	QList<long> idsOFCurrentMatch;
	QQueue<QList<int > > all_positions;
	QQueue<QList<long> > all_categories;
	QQueue<QList<long> > all_ids;
	all_categories.clear();
	all_positions.clear();
	all_ids.clear();
	all_categories.enqueue(catsOFCurrentMatch);
	all_positions.enqueue(sub_positionsOFCurrentMatch);
	all_ids.enqueue(idsOFCurrentMatch);
#ifdef REDUCE_THRU_DIACRITICS
	QList <QList< QString> > possible_raw_datasOFCurrentMatch;
	QQueue<QList<QList< QString> > > all_raw_datas;
	all_raw_datas.clear();
	all_raw_datas.enqueue(possible_raw_datasOFCurrentMatch);
#endif
#elif defined(PARENT)
	/*this->sub_positionsOFCurrentMatch.clear();
	this->catsOFCurrentMatch.clear();
	this->idsOFCurrentMatch.clear();*/
	filled_details=false;
#endif
	queue.enqueue((letter_node*)Tree->getFirstNode());
#ifdef QUEUE
	QList  <int > temp_partition;
	QList  <long> temp_ids;
	QList  <long> temp_categories;
#ifdef REDUCE_THRU_DIACRITICS
	QList <QList<QString> > temp_raw_datas;
#endif
#endif
	bool stop=false;
	int nodes_per_level=1;
	bool wait_for_dequeue=false;
	position=info.start;
	while (!queue.isEmpty() && !stop)
	{
		if (wait_for_dequeue)
			position++;
		wait_for_dequeue=false;
		node * current_node=queue.dequeue();
#ifdef QUEUE
		sub_positionsOFCurrentMatch=all_positions.dequeue();
		catsOFCurrentMatch =all_categories.dequeue();
		idsOFCurrentMatch=all_ids.dequeue();
#ifdef REDUCE_THRU_DIACRITICS
		possible_raw_datasOFCurrentMatch=all_raw_datas.dequeue();
#endif
#endif
		nodes_per_level--;
		if (nodes_per_level==0)
		{
			wait_for_dequeue=true;
			nodes_per_level=queue.count();
		}
#ifndef USE_TRIE_WALK
		QChar future_letter=info.text->at(position);
#else
		QChar future_letter;
		if (position==info.text->length())
			future_letter='\0';
		else if (position>info.text->length())
			break;
		else
		{
			future_letter=info.text->at(position);
			while (position <info.text->length() && isDiacritic(future_letter))
			{
				position++;
				if (position==info.text->length())
					future_letter='\0';
				else
					future_letter=info.text->at(position);
			}
		}
#endif

		letter_node * let_node=current_node->getLetterChild(future_letter);
		if (let_node!=NULL) {
			queue.enqueue(let_node);
#ifdef QUEUE
			temp_partition=sub_positionsOFCurrentMatch;
			temp_categories=catsOFCurrentMatch;
			temp_ids=idsOFCurrentMatch;
			all_positions.enqueue(temp_partition);
			all_categories.enqueue(temp_categories);
			all_ids.enqueue(temp_ids);
#ifdef REDUCE_THRU_DIACRITICS
			temp_raw_datas=possible_raw_datasOFCurrentMatch;
			all_raw_datas.enqueue(temp_raw_datas);
#endif
#endif
			if (wait_for_dequeue)
				nodes_per_level++;
		}
		QList<result_node *>* current_result_children=current_node->getResultChildren();
		int num_children=current_result_children->count();
		for (int j=0;j<num_children;j++)
		{
			result_node *current_child=current_result_children->at(j);
#ifdef QUEUE
			temp_partition=sub_positionsOFCurrentMatch;
			temp_categories=catsOFCurrentMatch;
			temp_ids=idsOFCurrentMatch;
			temp_partition.append(position-1);
			temp_categories.append(((result_node *)current_child)->get_previous_category_id());
			temp_ids.append(((result_node *)current_child)->get_affix_id());
			this->sub_positionsOFCurrentMatch=temp_partition;
			this->catsOFCurrentMatch=temp_categories;
			this->idsOFCurrentMatch=temp_ids;
#ifdef REDUCE_THRU_DIACRITICS
			temp_raw_datas=possible_raw_datasOFCurrentMatch;
			temp_raw_datas.append(((result_node *)current_child)->raw_datas);
			this->possible_raw_datasOFCurrentMatch=temp_raw_datas;
#endif
#endif
#if defined(PARENT)
			reached_node=current_child;
#endif
			resulting_category_idOFCurrentMatch=((result_node *)current_child)->get_resulting_category_id();
			bool isAccept=((result_node *)current_child)->is_accept_state();
			if ( isAccept && shouldcall_onmatch_ex(position) &&
				 !(on_match_helper())) {
					stop=true;
					break;
			} else {
				let_node=current_child->getLetterChild(future_letter);///
				if (let_node!=NULL)
				{
					queue.enqueue((letter_node*)let_node);
#ifdef QUEUE
					all_positions.enqueue(temp_partition);
					all_categories.enqueue(temp_categories);
					all_ids.enqueue(temp_ids);
#ifdef REDUCE_THRU_DIACRITICS
					all_raw_datas.enqueue(temp_raw_datas);
#endif
#endif
					if (wait_for_dequeue)
						nodes_per_level++;
				}
			}
		}
	}
	return (!stop);
}
bool TreeSearch::on_match_helper() {
	//check if matches with Diacritics
#ifdef PARENT
	filled_details=false;
	fill_details();
#endif
	int startPos=info.start, subpos, last;
#ifdef REDUCE_THRU_DIACRITICS
	if (reduce_thru_diacritics) {
		int count=sub_positionsOFCurrentMatch.count();
	#ifdef DEBUG
		out<<"<"<<position<<">\n";
	#endif
		for (int k=0;k<count;k++) {
			subpos=sub_positionsOFCurrentMatch[k];//getLastDiacritic(position-1,info.text)-1);
			QStringRef subword=addlastDiacritics(startPos,subpos, info.text, last);
			//qDebug() <<subword;
			for (int j=0;j<possible_raw_datasOFCurrentMatch[k].count();j++) {
				QString rawdata=possible_raw_datasOFCurrentMatch[k][j].getActual();
				if (rawdata.size()>0 && isDiacritic(rawdata[0])) {//in this case we can assume we are working in the first suffix or recursive affixes whose diacritics are for those before them
					QStringRef diacritics_of_word=getDiacriticsBeforePosition(startPos,info.text),
							   diacritics_of_rawdata=addlastDiacritics(0,0,&rawdata);//to get first couple of diacritics of raw_data without letters
				#ifdef DEBUG
					qDebug() <<diacritics_of_word<<"\t"<<diacritics_of_rawdata;
				#endif
					if (!equal(diacritics_of_word,diacritics_of_rawdata)) {
						possible_raw_datasOFCurrentMatch[k].removeAt(j);
						j--;
						continue;
					}
				}
			#ifdef DEBUG
				out<<"p-S:"<<k<<"<"<<sub_positionsOFCurrentMatch[k]<<">"<<"\t"<<subword.toString()<<"\t"<<possible_raw_datasOFCurrentMatch[k][j]<<"\n";
			#endif
				if (!equal(subword,rawdata)) {
					possible_raw_datasOFCurrentMatch[k].removeAt(j);
					j--;
				}
			}
			startPos=subpos+1;
		}
		for (int i=0;i<possible_raw_datasOFCurrentMatch.count();i++)
			if (0==possible_raw_datasOFCurrentMatch[i].count())
				return true; //not matching, continue without doing anything
	} else
		last=getLastDiacritic(position-1,info.text);
#else
	last=getLastDiacritic(position-1,info.text);
#endif
	info.finish=last-1;
	position=last;
	if (!onMatch())
		return false;
	else
		return true;
}

void TreeSearch::initializeAffixInfo(solution_position * sol_pos,int start_index) //zero and initialize solutions till 'last_index' exclusive
{
	minimal_item_info inf;
	int count=sub_positionsOFCurrentMatch.count();
	for (int i=start_index;i<count;i++)
	{
		inf.type=type;
		RawData & raw_data=possible_raw_datasOFCurrentMatch[i][0];
		if (multi_p.raw_data)
			inf.raw_data=raw_data.getActual();
		else
			inf.raw_data="";
		result_node * r_node=result_nodes->at(i);
		inf.category_id=r_node->get_previous_category_id();
		if (!multi_p.raw_dataONLY())
		{
			const ItemCatRaw2PosDescAbsMapItr & itr = map->find(ItemEntryKey(r_node->get_affix_id(),inf.category_id,raw_data.getOriginal()));
			assert(itr!=map->end());
			const ItemEntryInfo & ITRvalue=itr.value();
			if (multi_p.abstract_category)
				inf.abstract_categories=ITRvalue.first;
			else
				inf.abstract_categories=INVALID_BITSET;
			if (multi_p.description) {
				inf.abstract_categories=ITRvalue.first; //TODO: check if correct, added in case abstractcategory is reverse_description
				inf.description_id=ITRvalue.second;
			} else
				inf.description_id=-1;
			if (multi_p.POS)
				inf.POS=ITRvalue.third;
			else
				inf.POS="";
			if (i<sol_pos->indexes.count())
				sol_pos->indexes[i]= AffixPosition(0,itr);
			else
				sol_pos->indexes.insert(i, AffixPosition(0,itr));
		}
		else
		{
			inf.abstract_categories=INVALID_BITSET;
			inf.description_id=-1;
			inf.POS="";
			if (i<sol_pos->indexes.count())
				sol_pos->indexes[i]= AffixPosition(0,map->end());
			else
				sol_pos->indexes.insert(i, AffixPosition(0,map->end()));
		}
		if (affix_info.size()>i)
			affix_info[i]=inf;
		else
			affix_info.append(inf);
	}
	for (int i=count;i<sol_pos->indexes.count();i++)
		sol_pos->indexes.remove(i);
	sol_pos->store_solution(inf);//store the last bc it is the first to be modified (or accessed ??)
}
bool TreeSearch::increment(solution_position * info,int index)
{
	if (multi_p.NONE())
		return false;
	result_node * r_node=result_nodes->at(index);
	minimal_item_info & inf=affix_info[index];
	SolutionsCompare comp(multi_p);
	if (!multi_p.raw_dataONLY())
	{
		ItemCatRaw2PosDescAbsMapItr & itr=info->indexes[index].second;
		itr++;
		long id=r_node->get_affix_id(), catID=r_node->get_previous_category_id();
		int & raw_index=info->indexes[index].first;
		RawData & raw_data=possible_raw_datasOFCurrentMatch[index][raw_index];
		QString raw_data_string=raw_data.getActual();
		const ItemEntryKey & key=itr.key();
		if (itr == map->end() || key != ItemEntryKey(id,catID,raw_data_string) )
		{
			if (info->indexes[index].first<possible_raw_datasOFCurrentMatch[index].count()-1)//check for next time
			{
				raw_index++;
				inf.type=type;
				RawData & raw_data=possible_raw_datasOFCurrentMatch[index][raw_index];
				raw_data_string=raw_data.getActual();
				if (multi_p.raw_data)
					inf.raw_data=raw_data_string;
				else
					inf.raw_data="";
				inf.category_id=r_node->get_previous_category_id();
				itr = map->find(ItemEntryKey(r_node->get_affix_id(),inf.category_id,raw_data.getOriginal()));
			}
			else
			{
				if (index>0)
				{
					initializeAffixInfo(info,index);
					info->clear_stored_solutions();
					return increment(info,index-1);
				}
				else
					return false;
			}
		}
		if (multi_p.abstract_category)
			inf.abstract_categories=itr.value().first;
		else
			inf.abstract_categories=INVALID_BITSET;
		if (multi_p.description)
			inf.description_id=itr.value().second;
		else
			inf.description_id=-1;
		if (multi_p.POS)
			inf.POS=itr.value().third;
		else
			inf.POS="";
		//compare to previous solutions
		if (comp.found(info,inf))
			return increment(info,index);
		info->store_solution(inf);
	}
	else
	{
		if (info->indexes[index].first<possible_raw_datasOFCurrentMatch[index].count()-1)
		{
			int & raw_index=info->indexes[index].first;
			raw_index++;
			inf.type=type;
			inf.raw_data=possible_raw_datasOFCurrentMatch[index][raw_index].getActual();
			inf.category_id=r_node->get_previous_category_id();
			inf.abstract_categories=INVALID_BITSET;
			inf.description_id=-1;
			inf.POS="";
		}
		else
		{
			if (index>0)
			{
				initializeAffixInfo(info,index);
				info->clear_stored_solutions();
				return increment(info,index-1);
			}
			else
				return false;
		}
		//compare to previous solutions
		if (comp.found(info,inf))
			return increment(info,index);
		info->store_solution(inf);
	}
	return true;
}
