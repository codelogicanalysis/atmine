#include <QStringList>
#include "diacriticguidelines.h"

DiacriticGuidelines::DiacriticGuidelines(long number_of_solutions, bool get_all_details) : Enumerator(get_all_details) {
    solution_counter = 1;
    this->number_of_solutions = number_of_solutions;
    index = 0;
    trie = new VWTrie();
}

bool DiacriticGuidelines::saveTrie() {
    trie->save(QString(".vocalizedword_trie_bama.dat").toStdString().data());
}

bool DiacriticGuidelines::on_match()
{
    QString vocalizedWord;
    QString unvocalizedWord;

    /** Get vocalized and unvocalized words **/
    int prefix_infos_size = prefix_infos->size();
    for (int i= 0; i<prefix_infos_size;i++) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(!(pre.raw_data.isEmpty())) {
            vocalizedWord.append(pre.raw_data);
        }
    }

    vocalizedWord.append(stem_info->raw_data);

    int suffix_infos_size = suffix_infos->size();
    for (int i=0;i<suffix_infos_size;i++) {
        minimal_item_info & suff = (*suffix_infos)[i];
        if(!(suff.raw_data.isEmpty())) {
            vocalizedWord.append(suff.raw_data);
        }
    }

    unvocalizedWord = removeDiacritics(vocalizedWord);

    /** Add word to trie **/

    trie->store(unvocalizedWord,index);
    index++;

    /** Check for number of solutions requested **/
    if(number_of_solutions == -1) {
        return true;
    }
    else if(solution_counter != number_of_solutions) {
        solution_counter++;
        return true;
    }
    else {
        return false;
    }
};
