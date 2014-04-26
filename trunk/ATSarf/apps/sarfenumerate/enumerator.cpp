#include "logger.h"
#include "Search_by_item_locally.h"
#include "text_handling.h"
#include "diacritics.h"
#include "enumerator.h"
#include "test.h"
#include "inflections.h"

SarfParameters sarfParameters;

PrefixEnumeratorMachine::PrefixEnumeratorMachine(Enumerator * controller):PrefixEnumerate()
{
        this->controller=controller;
}

bool PrefixEnumeratorMachine::onMatch()
{
        if (controller->Stem!=NULL)
                delete controller->Stem;//TODO: change this allocation, deallocation to clear
        controller->Stem=new StemEnumeratorMachine(controller,controller->Prefix->resulting_category_idOFCurrentMatch);
        controller->Stem->setSolutionSettings(controller->multi_p);
        return (*controller->Stem)();
}

StemEnumeratorMachine::StemEnumeratorMachine(Enumerator * controller, long prefix_category):StemEnumerate(prefix_category)
{
        this->controller =controller;
}

bool StemEnumeratorMachine::onMatch()
{
        if (controller->called_everything) {
                if (controller->Suffix!=NULL)
                        delete controller->Suffix;//TODO: change this allocation, deallocation to clear
                controller->Suffix= new SuffixEnumeratorMachine(controller,controller->Prefix->resulting_category_idOFCurrentMatch,controller->Stem->category_of_currentmatch);
                controller->Suffix->setSolutionSettings(controller->multi_p);
                return (*controller->Suffix)();
        } else {
                if (!controller->on_match_helper())
                        return false;
        }
        return true;
}

SuffixEnumeratorMachine::SuffixEnumeratorMachine(Enumerator * controller, long prefix_category,long stem_category):SuffixEnumerate(prefix_category,stem_category)
{
    this->controller=controller;
}

bool SuffixEnumeratorMachine::onMatch()
{
    return controller->on_match_helper();
}

bool Enumerator::on_match_helper()
{
    // Set the finish index of the input as the finish index returned by the Suffix machine
    if(get_all_details) {
        solution_position * s_inf=Suffix->computeFirstSolution();
        do {
            solution_position * p_inf=Prefix->computeFirstSolution();
            do {
                solution_position * S_inf=Stem->computeFirstSolution();
                do {
                    suffix_infos=&Suffix->affix_info;
                    prefix_infos=&Prefix->affix_info;
                    stem_info=Stem->solution;

                    for (int j=0;j<2;j++) {
                        QVector<minimal_item_info> * infos=(j==0?prefix_infos:suffix_infos);
                        QList<result_node *> * results=(j==0?Prefix->result_nodes:Suffix->result_nodes);
                        for (int i=0;i<results->size();i++) {
                            QString inflectionRule=results->at(i)->getInflectionRule();
                            minimal_item_info & current=(*infos)[i];
                            if (i>1) {
                                minimal_item_info & previous=(*infos)[i-1];
                                applyInflections(inflectionRule,previous,current);
                            } else {
                                applyInflections(inflectionRule,current);
                            }
                        }
                    }

                    if (!on_match())
                            return false;
                }while (Stem->computeNextSolution(S_inf));
                delete S_inf;
            }while (Prefix->computeNextSolution(p_inf));
            delete p_inf;
        }while(Suffix->computeNextSolution(s_inf));
        delete s_inf;
    } else {
            if (!on_match())
                    return false;
    }
    return true;
}

bool Enumerator::on_match() {
        theSarf->out<<"ALTERNATIVE:\t";
        QString word;
        for (int i=0;i<prefix_infos->size();i++)
                word.append(prefix_infos->at(i).raw_data);
        word.append(stem_info->raw_data);
        for (int i=0;i<suffix_infos->size();i++)
                word.append(suffix_infos->at(i).raw_data);
        theSarf->out <<" "<<word<<"\n";

        for (int i=0;i<prefix_infos->size();i++) {
                minimal_item_info & pre = (*prefix_infos)[i];
                if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
                        continue;
                theSarf->out	<<"PREFIX:\t"
                        <<pre.raw_data<<"\t"
                        <<"\""<<pre.description()<<"\"\t"
                        <<pre.POS<<"\n";
        }

        minimal_item_info & stem = *stem_info;
        theSarf->out	<<"STEM:\t"
                <<stem.raw_data<<"\t"
                <<"\""<<stem.description()<<"\"\t"
                <<stem.POS<<"\n";

        for (int i=0;i<suffix_infos->size();i++) {
                minimal_item_info & suff = (*suffix_infos)[i];
                if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                        continue;
                theSarf->out	<<"SUFFIX:\t"
                        <<suff.raw_data<<"\t"
                        <<"\""<<suff.description()<<"\"\t"
                        <<suff.POS<<"\n";
        }
        theSarf->out<<"\n";

        return true;
}
