#include "tree_enumerate.h"
#include "logger.h"
#include "suffix_enumerate.h"
#include "stem_enumerate.h"
#include "prefix_enumerate.h"
#include <QVector>
#include "node.h"
#include "tree.h"
#include "text_handling.h"
#include "diacritics.h"
#include "database_info_block.h"
#include <QDebug>
#include <assert.h>
#include <QString>

void TreeEnumerate::fill_details()
{
    if (!filled_details) {
        possible_raw_datasOFCurrentMatch.clear();
        possible_raw_datasOFCurrentMatch.prepend(((result_node *)reached_node)->raw_datas);

        letter_node * tree_head=(letter_node*)Tree->getFirstNode();
        node * current_parent=((result_node *)reached_node)->parent;
        assert(current_parent->getResultChildren()->contains(reached_node));

        while (current_parent!=tree_head) {
            if (!(current_parent->isLetterNode())) {
                possible_raw_datasOFCurrentMatch.prepend(((result_node *)current_parent)->raw_datas);
            }
            current_parent=current_parent->parent;
        }
        filled_details=true;
    }
}

inline void addLetterToQueue(QQueue<letter_node *> & queue, node * current_node) {
    QVector<letter_node* > current_letter_children = current_node->getLetterChildren();
    int num_letter = current_letter_children.count();
    for(int i=0; i< num_letter; i++) {
        if(current_letter_children[i] != NULL) {
            queue.enqueue(current_letter_children[i]);
        }
    }
    return;
}

bool TreeEnumerate::operator ()()
{
    QQueue<letter_node *> queue;
    queue.clear();

    filled_details=false;
    queue.enqueue((letter_node*)Tree->getFirstNode());
    bool stop=false;
    while ((!queue.isEmpty())  && !stop) {
        node * current_node=NULL;
        current_node=queue.dequeue();
        addLetterToQueue(queue,current_node);

        QList<result_node *>* current_result_children=current_node->getResultChildren();
        int num_children=current_result_children->count();
        for (int j=0;j<num_children;j++) {
            result_node *current_child=current_result_children->at(j);
            reached_node=current_child;
            resulting_category_idOFCurrentMatch=((result_node *)current_child)->get_resulting_category_id();
            bool isAccept=((result_node *)current_child)->is_accept_state();
            if ( isAccept && shouldcall_onmatch_ex() && !(on_match_helper())) {
                stop=true;
                break;
            } else {
                addLetterToQueue(queue,current_child);
            }
        }
    }
    return (!stop);
}
bool TreeEnumerate::on_match_helper() {
    //check if matches with Diacritics
    filled_details=false;
    fill_details();

    if (!onMatch())
            return false;
    else
            return true;
}

void TreeEnumerate::initializeAffixInfo(solution_position * sol_pos,int start_index) { //zero and initialize solutions till 'last_index' exclusive
        minimal_item_info inf;
        int count = this->possible_raw_datasOFCurrentMatch.count();
        for (int i=start_index;i<count;i++) {
                inf.type=type;
                RawData & raw_data=possible_raw_datasOFCurrentMatch[i][0];
                if (multi_p.raw_data)
                        inf.raw_data=raw_data.getActual();
                else
                        inf.raw_data="";
                result_node * r_node=result_nodes->at(i);
                inf.category_id=r_node->get_previous_category_id();
                if (!multi_p.raw_dataONLY()) {
                        const ItemCatRaw2AbsDescPosMapItr & itr = map->find(ItemEntryKey(r_node->get_affix_id(),inf.category_id,raw_data.getOriginal()));
                        assert(itr!=map->end());
                        const ItemEntryInfo & ITRvalue=itr.value();
                        if (multi_p.abstract_category)
                                inf.abstract_categories=ITRvalue.first;
                        else
                                inf.abstract_categories=INVALID_BITSET;
                        if (multi_p.description) {
                                inf.abstract_categories=ITRvalue.first; //TODO: check if correct, added in case abstractcategory is reverse_description
                                inf.setDescription(ITRvalue.second);
                        } else
                                inf.setDescription(-1);
                        if (multi_p.POS)
                                inf.POS=ITRvalue.third;
                        else
                                inf.POS="";
                        if (i<sol_pos->indexes.count())
                                sol_pos->indexes[i]= AffixPosition(0,itr);
                        else
                                sol_pos->indexes.insert(i, AffixPosition(0,itr));
                } else {
                        inf.abstract_categories=INVALID_BITSET;
                        inf.setDescription(-1);
                        inf.POS="";
                        if (i<sol_pos->indexes.count())
                                sol_pos->indexes[i]= AffixPosition(0,map->end());
                        else
                                sol_pos->indexes.insert(i, AffixPosition(0,map->end()));
                }
                if (affix_info.size()>i){
                        affix_info[i]=inf;
                } else {
                        affix_info.append(inf);
                }
        }
        for (int i=count;i<sol_pos->indexes.count();i++)
                sol_pos->indexes.remove(i);
        sol_pos->store_solution(inf);//store the last bc it is the first to be modified (or accessed ??)
}
bool TreeEnumerate::increment(solution_position * info,int index) {
        if (multi_p.NONE())
                return false;
        result_node * r_node=result_nodes->at(index);
        minimal_item_info & inf=affix_info[index];
        SolutionsCompare comp(multi_p);
        if (!multi_p.raw_dataONLY()) {
                ItemCatRaw2AbsDescPosMapItr & itr=info->indexes[index].second;
                itr++;
                long id=r_node->get_affix_id(), catID=r_node->get_previous_category_id();
                int & raw_index=info->indexes[index].first;
                RawData & raw_data=possible_raw_datasOFCurrentMatch[index][raw_index];
                QString raw_data_string=raw_data.getActual();
                const ItemEntryKey & key=itr.key();
                if (itr == map->end() || key != ItemEntryKey(id,catID,raw_data.getOriginal()) ) {
                        if (info->indexes[index].first<possible_raw_datasOFCurrentMatch[index].count()-1) {//check for next time
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
                        } else {
                                if (index>0) {
                                        initializeAffixInfo(info,index);
                                        info->clear_stored_solutions();
                                        return increment(info,index-1);
                                } else
                                        return false;
                        }
                }
                if (multi_p.abstract_category)
                        inf.abstract_categories=itr.value().first;
                else
                        inf.abstract_categories=INVALID_BITSET;
                if (multi_p.description)
                        inf.setDescription(itr.value().second);
                else
                        inf.setDescription(-1);
                if (multi_p.POS)
                        inf.POS=itr.value().third;
                else
                        inf.POS="";
                //compare to previous solutions
                if (comp.found(info,inf))
                        return increment(info,index);
                info->store_solution(inf);
        } else {
                if (info->indexes[index].first<possible_raw_datasOFCurrentMatch[index].count()-1)
                {
                        int & raw_index=info->indexes[index].first;
                        raw_index++;
                        inf.type=type;
                        inf.raw_data=possible_raw_datasOFCurrentMatch[index][raw_index].getActual();
                        inf.category_id=r_node->get_previous_category_id();
                        inf.abstract_categories=INVALID_BITSET;
                        inf.setDescription(-1);
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
