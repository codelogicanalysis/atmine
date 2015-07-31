/**
  * @file   tree_search.cpp
  * @author Jad Makhlouta and documented by Ameen Jaber
  */
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

void TreeSearch::fill_details() { //this function fills the public member functions such as QList<int> sub_positionsOFCurrentMatch & QList<long> catsOFCurrentMatch;
    if (!filled_details) {
        sub_positionsOFCurrentMatch.clear();
        #ifndef MULTIPLICATION
        catsOFCurrentMatch.clear();
        idsOFCurrentMatch.clear();
        catsOFCurrentMatch.insert(0, ((result_node *)reached_node)->get_previous_category_id());
        idsOFCurrentMatch.insert(0, ((result_node *)reached_node)->get_affix_id());
        #endif
        possible_raw_datasOFCurrentMatch.clear();
        possible_raw_datasOFCurrentMatch.prepend(((result_node *)reached_node)->raw_datas);
        //qDebug()<<position<<":"<<((result_node *)reached_node)->raw_datas[0];
        int index = getPositionOneLetterBackward(position, info.text);
        sub_positionsOFCurrentMatch.prepend(index);
        letter_node *tree_head = (letter_node *)Tree->getFirstNode();
        node *current_parent = ((result_node *)reached_node)->parent;
        assert(current_parent->getResultChildren()->contains(reached_node));

        while (current_parent != tree_head) {
            if (current_parent->isLetterNode()) {
                index = getLastLetter_index(*info.text, index);

                if (((letter_node *)current_parent)->getLetter() != '\0') {
                    index--;
                }
            } else {
                #ifndef MULTIPLICATION
                catsOFCurrentMatch.insert(0, ((result_node *)current_parent)->get_previous_category_id());
                idsOFCurrentMatch.insert(0, ((result_node *)current_parent)->get_affix_id());//was : reached_node
                #endif
                sub_positionsOFCurrentMatch.prepend(index);
                possible_raw_datasOFCurrentMatch.prepend(((result_node *)current_parent)->raw_datas);
                //qDebug()<<position-count<<":"<<((result_node *)current_parent)->raw_datas[0];
            }

            current_parent = current_parent->parent;
        }

        //qDebug()<<"--";
        filled_details = true;
    }
}

inline bool addLetterToQueue(QQueue<letter_node *> &queue, QQueue<letter_node *> &queue_emptyCharacters,
                             node *current_node, QChar future_letter) {
    bool added = false;
    letter_node *let_node = current_node->getLetterChild(future_letter);

    if (let_node != NULL) {
        queue.enqueue(let_node);
        added = true;
    }

    if (future_letter != '\0') { //just needed to traverse also the empty character always
        QChar l = '\0';
        let_node = current_node->getLetterChild(l);

        if (let_node != NULL) {
            queue_emptyCharacters.enqueue(let_node);
        }
    }

    return added;
}

bool TreeSearch::operator()() {
    QQueue<letter_node *> queue;
    QQueue<letter_node *> queue_emptyCharacters;
    queue.clear();
    filled_details = false;
    queue.enqueue((letter_node *)Tree->getFirstNode());
    bool stop = false;
    int nodes_per_level = 1;
    bool wait_for_dequeue = false;
    position = info.start;

    while ((!queue.isEmpty() || !queue_emptyCharacters.isEmpty())  && !stop) {
        node *current_node = NULL;

        if (wait_for_dequeue) {
            if (!queue_emptyCharacters.isEmpty()) {
                current_node = queue_emptyCharacters.dequeue();
            } else {
                wait_for_dequeue = false;
                position++;
            }
        }

        if (current_node == NULL) {
            current_node = queue.dequeue();
            nodes_per_level--;

            if (nodes_per_level == 0) {
                wait_for_dequeue = true;
                nodes_per_level = queue.count();
            }
        }

        QChar future_letter;

        if (position == info.text->length()) {
            future_letter = '\0';
        } else if (position > info.text->length()) {
            future_letter = '\0';
            position = info.text->length();
            //break;
        } else {
            future_letter = info.text->at(position);

            while (position < info.text->length() && isDiacritic(future_letter)) {
                position++;

                if (position == info.text->length()) {
                    future_letter = '\0';
                } else {
                    future_letter = info.text->at(position);
                }
            }
        }

        bool added_to_main_queue = addLetterToQueue(queue, queue_emptyCharacters, current_node, future_letter);

        if (added_to_main_queue && wait_for_dequeue) {
            nodes_per_level++;
        }

        QList<result_node *> *current_result_children = current_node->getResultChildren();
        int num_children = current_result_children->count();

        for (int j = 0; j < num_children; j++) {
            result_node *current_child = current_result_children->at(j);
            reached_node = current_child;
            resulting_category_idOFCurrentMatch = ((result_node *)current_child)->get_resulting_category_id();
            bool isAccept = ((result_node *)current_child)->is_accept_state();

            if (isAccept && shouldcall_onmatch_ex(position) &&
                !(on_match_helper())) {
                stop = true;
                break;
            } else {
                bool added_to_main_queue = addLetterToQueue(queue, queue_emptyCharacters, current_child, future_letter);

                if (added_to_main_queue && wait_for_dequeue) {
                    nodes_per_level++;
                }
            }
        }
    }

    return (!stop);
}
bool TreeSearch::on_match_helper() {
    //check if matches with Diacritics
    filled_details = false;
    fill_details();
    int startPos = info.start, subpos, last;

    if (reduce_thru_diacritics) {
        int count = sub_positionsOFCurrentMatch.count();

        for (int k = 0; k < count; k++) {
            subpos = sub_positionsOFCurrentMatch[k]; //getLastDiacritic(position-1,info.text)-1);
            QStringRef subword = addlastDiacritics(startPos, subpos, info.text, last);

            //qDebug() <<subword;
            for (int j = 0; j < possible_raw_datasOFCurrentMatch[k].count(); j++) {
                QString rawdata = possible_raw_datasOFCurrentMatch[k][j].getActual();
                bool raw_has_diacritics = startPos > 0 && isDiacritic((*info.text)[startPos - 1]);
                bool input_ended_with_diacritic = subword.size() > 0 && isDiacritic(subword.at(subword.size() - 1));

                if (raw_has_diacritics ||
                    input_ended_with_diacritic) {//in this case we can assume we are working in the first suffix or recursive affixes whose diacritics are for those before them
                    QStringRef diacritics_of_word = getDiacriticsBeforePosition(startPos, info.text),
                               diacritics_of_rawdata = (rawdata.size() > 0 ? addlastDiacritics(0, 0,
                                                        &rawdata) : QStringRef()); //to get first couple of diacritics of raw_data without letters

                    if (!equal(diacritics_of_word, diacritics_of_rawdata, true)) { //force_shadde
                        possible_raw_datasOFCurrentMatch[k].removeAt(j);
                        j--;
                        continue;
                    }
                }

                if (!equal(subword, rawdata, true)) { //force_shadde
                    possible_raw_datasOFCurrentMatch[k].removeAt(j);
                    j--;
                }
            }

            startPos = subpos + 1;
        }

        for (int i = 0; i < possible_raw_datasOFCurrentMatch.count(); i++)
            if (0 == possible_raw_datasOFCurrentMatch[i].count()) {
                return true;    //not matching, continue without doing anything
            }
    } else {
        last = getLastDiacritic(position - 1, info.text);
    }

    info.finish = last - 1;
    position = last;

    if (!onMatch()) {
        return false;
    } else {
        return true;
    }
}

void TreeSearch::initializeAffixInfo(solution_position *sol_pos,
                                     int start_index) { //zero and initialize solutions till 'last_index' exclusive
    minimal_item_info inf;
    int count = sub_positionsOFCurrentMatch.count();

    for (int i = start_index; i < count; i++) {
        inf.type = type;
        RawData &raw_data = possible_raw_datasOFCurrentMatch[i][0];

        if (multi_p.raw_data) {
            inf.raw_data = raw_data.getActual();
        } else {
            inf.raw_data = "";
        }

        result_node *r_node = result_nodes->at(i);
        inf.category_id = r_node->get_previous_category_id();

        if (!multi_p.raw_dataONLY()) {
            const ItemCatRaw2AbsDescPosMapItr &itr = map->find(ItemEntryKey(r_node->get_affix_id(), inf.category_id,
                                                     raw_data.getOriginal()));
            assert(itr != map->end());
            const ItemEntryInfo &ITRvalue = itr.value();

            if (multi_p.abstract_category) {
                inf.abstract_categories = ITRvalue.first;
            } else {
                inf.abstract_categories = INVALID_BITSET;
            }

            if (multi_p.description) {
                inf.abstract_categories =
                    ITRvalue.first; //TODO: check if correct, added in case abstractcategory is reverse_description
                inf.setDescription(ITRvalue.second);
            } else {
                inf.setDescription(-1);
            }

            if (multi_p.POS) {
                inf.POS = ITRvalue.third;
            } else {
                inf.POS = "";
            }

            if (i < sol_pos->indexes.count()) {
                sol_pos->indexes[i] = AffixPosition(0, itr);
            } else {
                sol_pos->indexes.insert(i, AffixPosition(0, itr));
            }
        } else {
            inf.abstract_categories = INVALID_BITSET;
            inf.setDescription(-1);
            inf.POS = "";

            if (i < sol_pos->indexes.count()) {
                sol_pos->indexes[i] = AffixPosition(0, map->end());
            } else {
                sol_pos->indexes.insert(i, AffixPosition(0, map->end()));
            }
        }

        if (affix_info.size() > i) {
            affix_info[i] = inf;
        } else {
            affix_info.append(inf);
        }
    }

    for (int i = count; i < sol_pos->indexes.count(); i++) {
        sol_pos->indexes.remove(i);
    }

    sol_pos->store_solution(inf);//store the last bc it is the first to be modified (or accessed ??)
}
bool TreeSearch::increment(solution_position *info, int index) {
    if (multi_p.NONE()) {
        return false;
    }

    result_node *r_node = result_nodes->at(index);
    minimal_item_info &inf = affix_info[index];
    SolutionsCompare comp(multi_p);

    if (!multi_p.raw_dataONLY()) {
        ItemCatRaw2AbsDescPosMapItr &itr = info->indexes[index].second;
        itr++;
        long id = r_node->get_affix_id(), catID = r_node->get_previous_category_id();
        int &raw_index = info->indexes[index].first;
        RawData &raw_data = possible_raw_datasOFCurrentMatch[index][raw_index];
        QString raw_data_string = raw_data.getActual();
        const ItemEntryKey &key = itr.key();

        if (itr == map->end() || key != ItemEntryKey(id, catID, raw_data.getOriginal())) {
            if (info->indexes[index].first < possible_raw_datasOFCurrentMatch[index].count() - 1) { //check for next time
                raw_index++;
                inf.type = type;
                RawData &raw_data = possible_raw_datasOFCurrentMatch[index][raw_index];
                raw_data_string = raw_data.getActual();

                if (multi_p.raw_data) {
                    inf.raw_data = raw_data_string;
                } else {
                    inf.raw_data = "";
                }

                inf.category_id = r_node->get_previous_category_id();
                itr = map->find(ItemEntryKey(r_node->get_affix_id(), inf.category_id, raw_data.getOriginal()));
            } else {
                if (index > 0) {
                    initializeAffixInfo(info, index);
                    info->clear_stored_solutions();
                    return increment(info, index - 1);
                } else {
                    return false;
                }
            }
        }

        if (multi_p.abstract_category) {
            inf.abstract_categories = itr.value().first;
        } else {
            inf.abstract_categories = INVALID_BITSET;
        }

        if (multi_p.description) {
            inf.setDescription(itr.value().second);
        } else {
            inf.setDescription(-1);
        }

        if (multi_p.POS) {
            inf.POS = itr.value().third;
        } else {
            inf.POS = "";
        }

        //compare to previous solutions
        if (comp.found(info, inf)) {
            return increment(info, index);
        }

        info->store_solution(inf);
    } else {
        if (info->indexes[index].first < possible_raw_datasOFCurrentMatch[index].count() - 1) {
            int &raw_index = info->indexes[index].first;
            raw_index++;
            inf.type = type;
            inf.raw_data = possible_raw_datasOFCurrentMatch[index][raw_index].getActual();
            inf.category_id = r_node->get_previous_category_id();
            inf.abstract_categories = INVALID_BITSET;
            inf.setDescription(-1);
            inf.POS = "";
        } else {
            if (index > 0) {
                initializeAffixInfo(info, index);
                info->clear_stored_solutions();
                return increment(info, index - 1);
            } else {
                return false;
            }
        }

        //compare to previous solutions
        if (comp.found(info, inf)) {
            return increment(info, index);
        }

        info->store_solution(inf);
    }

    return true;
}
