/**
  * @file   stem_search.cpp
  * @author Jad Makhlouta and documented by Ameen Jaber
  * @brief
  */
#include "Search_by_item.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stem_search.h"
#include "text_handling.h"
#include <assert.h>

bool StemSearch::check_for_terminal(int letter_index, ATTrie::Position pos) {
    const StemNode *node = NULL;
    Search_StemNode s1;

    if (trie->isTerminal(pos)) {
        ATTrie::Position pos2 = trie->clonePosition(pos);
        trie->walk(pos2, '\0');
        node = trie->getData(pos2);
        trie->freePosition(pos2);

        if (node != NULL) {
            id_of_currentmatch = node->stem_id;
            s1.setNode(node);
        } else {
            return true;
        }
    } else {
        return true;
    }

    if (!on_match_helper(letter_index, s1)) {
        stop = true;
        return false;
    }

    return true;
}

void StemSearch::traverse(int letter_index, ATTrie::Position pos) {
    int length = info.text->length();

    for (int i = max(letter_index, 0); i < length && !stop; i++) {
        QChar current_letter = info.text->at(i);

        //qDebug()<<"s:"<<current_letter;
        if (isDiacritic(current_letter)) {
            continue;
        }

        int lastNonDiacriticLetterIndex = getLastLetter_index(*info.text, i - 1);

        if (lastNonDiacriticLetterIndex >= 0) {
            QChar ch = info.text->at(lastNonDiacriticLetterIndex);

            if (isNonConnectingLetter(ch)) {
                if (trie->isWalkable(pos, ' ')) {
                    ATTrie::Position pos2 = trie->clonePosition(pos);
                    trie->walk(pos2, ' ');

                    if (!check_for_terminal(i - 1, pos2)) {
                        break;
                    }

                    traverse(i, pos2); //stay at same position bc it is previous nonConnectingLetter effect and not a new letter
                    trie->freePosition(pos2);

                    if (stop) {
                        break;    // to stop in case the previous traversal was required to stop
                    }
                }
            }
        }

        if (current_letter != alef) { //(!alefs.contains(current_letter) || info.start!=letter_index) { //
            if (!trie->walk(pos, current_letter)) {
                break;
            }

            if (!check_for_terminal(i, pos)) {
                break;
            }
        } else  {
            for (int j = 0; j < alefs.size(); j++) {
                if (!stop && trie->isWalkable(pos, alefs[j])) {
                    ATTrie::Position pos2 = trie->clonePosition(pos);
                    trie->walk(pos2, alefs[j]);

                    if (!check_for_terminal(i, pos2)) {
                        break;
                    }

                    traverse(i + 1, pos2);
                    trie->freePosition(pos2);
                }
            }

            break;
        }
    }
}

bool StemSearch::on_match_helper(int last_letter_index, Search_StemNode &s1) {
    int last;
    QStringRef subword = addlastDiacritics(info.start, last_letter_index, info.text, last);
    currentMatchPos = last > 0 ? last - 1 : 0;
    info.finish = currentMatchPos;

    //I think this is a more efficient implementation, less copying happening in this type of "retrieve"
    while (s1.retrieve(category_of_currentmatch, possible_raw_datas)) {
        if (isPrefixStemCompatible()) {
            if (!reduce_thru_diacritics) {
                if (!onMatch()) {
                    return false;
                }
            } else {
                for (int i = 0; i < possible_raw_datas.count(); i++) {
                    if (!equal(subword, possible_raw_datas[i], true, false)) { //force_shadde=true
                        possible_raw_datas.remove(i);
                        i--;
                    }
                }

                if (possible_raw_datas.count() > 0)
                    if (!onMatch()) {
                        return false;
                    }
            }
        }
    }

    return true;
}

solution_position *StemSearch::computeFirstSolution() {
    if (solution == NULL) {
        solution = new minimal_item_info;
    }

    solution_position *first = new solution_position();
    solution->type = STEM;

    if (multi_p.raw_data) {
        solution->raw_data = possible_raw_datas[0];
    } else {
        solution->raw_data = "";
    }

    solution->category_id = category_of_currentmatch;

    if (!multi_p.raw_dataONLY()) {
        ItemEntryKey key(id_of_currentmatch, category_of_currentmatch, possible_raw_datas[0]);
        ItemCatRaw2AbsDescPosMapItr itr = database_info.map_stem->find(key);

        if (itr == database_info.map_stem->end()) {
            qDebug() << "Stem Map Error, Not found:\t(" << id_of_currentmatch << ", " << category_of_currentmatch << ", " <<
                     possible_raw_datas[0] << ")";
            assert(itr != database_info.map_stem->end());
        }

        if (multi_p.abstract_category) {
            solution->abstract_categories = itr.value().first;
        } else {
            solution->abstract_categories = INVALID_BITSET;
        }

        if (multi_p.description) {
            solution->setDescription(itr.value().second);
        } else {
            solution->setDescription(-1);
        }

        if (multi_p.POS) {
            solution->POS = itr.value().third;
        } else {
            solution->POS = "";
        }

        first->indexes.append(AffixPosition(0, itr));
    } else {
        solution->abstract_categories = INVALID_BITSET;
        solution->setDescription(-1);
        solution->POS = "";
        first->indexes.append(AffixPosition(0, database_info.map_stem->end()));
    }

    first->store_solution(*solution);
    return first;
}

bool StemSearch::computeNextSolution(solution_position *current) { //compute next posibility
    if (multi_p.NONE()) {
        return false;
    }

    SolutionsCompare comp(multi_p);

    if (!multi_p.raw_dataONLY()) {
        ItemCatRaw2AbsDescPosMapItr &itr = current->indexes[0].second;
        itr++;
        QString raw_data = possible_raw_datas[current->indexes[0].first];
        ItemEntryKey key = itr.key();
        ItemEntryKey k1(id_of_currentmatch, category_of_currentmatch, raw_data);

        if (itr == database_info.map_stem->end() || key != k1) {
            if (current->indexes[0].first < possible_raw_datas.count() - 1) { //check for next time
                current->indexes[0].first++;
                solution->type = STEM;

                if (multi_p.raw_data) {
                    solution->raw_data = possible_raw_datas[current->indexes[0].first];
                } else {
                    solution->raw_data = "";
                }

                solution->category_id = category_of_currentmatch;
                ItemEntryKey k2(id_of_currentmatch, category_of_currentmatch, possible_raw_datas[current->indexes[0].first]);
                itr = database_info.map_stem->find(k2);
            } else {
                //current->clear_stored_solutions();
                return false;
            }
        }

        if (multi_p.abstract_category) {
            solution->abstract_categories = itr.value().first;
        } else {
            solution->abstract_categories = INVALID_BITSET;
        }

        if (multi_p.description) {
            solution->setDescription(itr.value().second);
        } else {
            solution->setDescription(-1);
        }

        if (multi_p.POS) {
            solution->POS = itr.value().third;
        } else {
            solution->POS = "";
        }

        //compare to previous solutions
        if (comp.found(current, *solution)) {
            return computeNextSolution(current);
        }

        current->store_solution(*solution);
    } else {
        if (current->indexes[0].first < possible_raw_datas.count() - 1) {
            current->indexes[0].first++;
            solution->type = STEM;
            solution->raw_data = possible_raw_datas[current->indexes[0].first];
            solution->category_id = category_of_currentmatch;
            solution->abstract_categories = INVALID_BITSET;
            solution->setDescription(-1);
            solution->POS = "";
        } else {
            //current->clear_stored_solutions();
            return false;
        }

        //compare to previous solutions
        if (comp.found(current, *solution)) {
            return computeNextSolution(current);
        }

        current->store_solution(*solution);
    }

    return true;
}
