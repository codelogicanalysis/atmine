#include "Search_by_item.h"
#include "text_handling.h"
#include "diacritics.h"
//#include "stem_enumerate.h"
#include "enumerator.h"
#include "text_handling.h"
#include <assert.h>
#include <QQueue>

StemEnumerate::StemEnumerate(long prefix_category)
{
    this->prefix_category=prefix_category;
    // Setting the trie pointer to the databased extracted data
    trie=database_info.Stem_Trie;
    solution=NULL;
    multi_p=M_ALL;
}

bool StemEnumerate::operator()()
{
    ATTrie::Position pos = trie->startWalk();
    stop=false;
    traverse(pos);
    return !stop;
}

void StemEnumerate::setSolutionSettings(multiply_params params)
{
        multi_p=params;
}

bool StemEnumerate::isPrefixStemCompatible() const
{
    //check rules AB
    compatibility_rules * cr= database_info.comp_rules;
    return ((*cr)(prefix_category,category_of_currentmatch));
}

void StemEnumerate::check_for_terminal(ATTrie::Position pos)
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
        else {
            return;
        }
    }
    else {
        return;
    }
    if (!on_match_helper(s1))
    {
        stop=true;
        return;
    }
    return;
}

void StemEnumerate::traverse(ATTrie::Position pos)
{

    QChar jeem = QChar(0x062C);
    QChar ghain = QChar(0x063A);
    QChar kaf = QChar(0x0643);
    QVector<QChar> letters;
    letters << hamza << alef_madda_above << alef_hamza_above << waw_hamza_above
            << alef_hamza_below << ya2_hamza_above << alef << ba2 << ta2_marbouta
            << ta2 << tha2 << jeem << _7a2 << kha2 << dal << thal << ra2 << zain
            << seen << sheen << sad << dad << tah << zah << _3yn << ghain << feh
            << qaf << kaf << lam << meem << noon << ha2 << waw << alef_maksoura
            << ya2 << alef_wasla;

    QQueue<ATTrie::Position> queue;
    queue.enqueue(pos);
    while(!queue.isEmpty()) {
        ATTrie::Position current_pos = queue.dequeue();

        for(int i=0; (i< letters.count()) & (!stop); i++) {
            QChar current_letter = letters[i];
            if (trie->isWalkable(current_pos,current_letter)) {
                ATTrie::Position temp_pos = trie->clonePosition(current_pos);
                trie->walk(temp_pos, current_letter);
                queue.enqueue(temp_pos);

                check_for_terminal(temp_pos);

                const StemNode * node=NULL;
                if (trie->isTerminal(temp_pos)) {
                        ATTrie::Position pos2 = trie->clonePosition(temp_pos);
                        trie->walk(pos2, '\0');
                        node = trie->getData(pos2);
                        trie->freePosition(pos2);
                        if (node != NULL) {
                        }
                }
            }
        }
        trie->freePosition(current_pos);
    }
}

bool StemEnumerate::on_match_helper(Search_StemNode & s1)
{
    while(s1.retrieve(category_of_currentmatch,possible_raw_datas)) {
        if (isPrefixStemCompatible()) {
            if (!onMatch())
                    return false;
        }
    }
    return true;
}

solution_position * StemEnumerate::computeFirstSolution()
{
        if (solution==NULL)
                solution=new minimal_item_info;
        solution_position * first=new solution_position();

        solution->type=STEM;
        if (multi_p.raw_data)
                solution->raw_data=possible_raw_datas[0];
        else
                solution->raw_data="";
        solution->category_id=category_of_currentmatch;
        if (!multi_p.raw_dataONLY())
        {
                ItemEntryKey key(id_of_currentmatch,category_of_currentmatch,possible_raw_datas[0]);
                ItemCatRaw2AbsDescPosMapItr itr = database_info.map_stem->find(key);
                if (itr==database_info.map_stem->end()) {
                        qDebug() <<"Stem Map Error, Not found:\t("<<id_of_currentmatch<<", "<<category_of_currentmatch<<", "<<possible_raw_datas[0]<<")";
                        assert(itr!=database_info.map_stem->end());
                }
                if (multi_p.abstract_category)
                        solution->abstract_categories=itr.value().first;
                else
                        solution->abstract_categories=INVALID_BITSET;
                if (multi_p.description)
                        solution->setDescription(itr.value().second);
                else
                        solution->setDescription(-1);
                if (multi_p.POS)
                        solution->POS=itr.value().third;
                else
                        solution->POS="";
                first->indexes.append( AffixPosition(0,itr));
        }
        else
        {
                solution->abstract_categories=INVALID_BITSET;
                solution->setDescription(-1);
                solution->POS="";
                first->indexes.append(AffixPosition(0,database_info.map_stem->end()));
        }
        first->store_solution(*solution);
        return first;
}

bool StemEnumerate::computeNextSolution(solution_position * current)//compute next posibility
{
        if (multi_p.NONE())
                return false;
        SolutionsCompare comp(multi_p);
        if (!multi_p.raw_dataONLY())
        {
                ItemCatRaw2AbsDescPosMapItr & itr=current->indexes[0].second;
                itr++;
                QString raw_data=possible_raw_datas[current->indexes[0].first];
                ItemEntryKey key=itr.key();
                ItemEntryKey k1(id_of_currentmatch,category_of_currentmatch,raw_data);
                if (itr == database_info.map_stem->end() || key != k1 ) {
                        if (current->indexes[0].first<possible_raw_datas.count()-1) {//check for next time
                                current->indexes[0].first++;
                                solution->type=STEM;
                                if (multi_p.raw_data)
                                        solution->raw_data=possible_raw_datas[current->indexes[0].first];
                                else
                                        solution->raw_data="";
                                solution->category_id=category_of_currentmatch;
                                ItemEntryKey k2(id_of_currentmatch,category_of_currentmatch,possible_raw_datas[current->indexes[0].first]);
                                itr = database_info.map_stem->find(k2);
                        } else {
                                //current->clear_stored_solutions();
                                return false;
                        }
                }
                if (multi_p.abstract_category)
                        solution->abstract_categories=itr.value().first;
                else
                        solution->abstract_categories=INVALID_BITSET;
                if (multi_p.description)
                        solution->setDescription(itr.value().second);
                else
                        solution->setDescription(-1);
                if (multi_p.POS)
                        solution->POS=itr.value().third;
                else
                        solution->POS="";
                //compare to previous solutions
                if (comp.found(current,*solution))
                        return computeNextSolution(current);
                current->store_solution(*solution);
        } else {
                if (current->indexes[0].first<possible_raw_datas.count()-1) {
                        current->indexes[0].first++;
                        solution->type=STEM;
                        solution->raw_data=possible_raw_datas[current->indexes[0].first];
                        solution->category_id=category_of_currentmatch;
                        solution->abstract_categories=INVALID_BITSET;
                        solution->setDescription(-1);
                        solution->POS="";
                } else {
                        //current->clear_stored_solutions();
                        return false;
                }
                //compare to previous solutions
                if (comp.found(current,*solution))
                        return computeNextSolution(current);
                current->store_solution(*solution);
        }
        return true;
}
