#include <QStringList>
#include "diacriticguidelines.h"

DiacriticGuidelines::DiacriticGuidelines(long number_of_solutions, enumeration_type enum_type, bool get_all_details) :
        Enumerator(enum_type, get_all_details) {
    solution_counter = 1;
    this->number_of_solutions = number_of_solutions;
    this->enum_type = enum_type;
    //trie = new VWTrie();
}

/*
bool DiacriticGuidelines::saveTrie() {
    trie->save(QString("unvocalizedword_trie_bama.dat").toStdString().data());
    return true;
}
*/

bool DiacriticGuidelines::serializeHash() {
    QString fileName = "unvocalized_words_";
    if(number_of_solutions == -1) {
        fileName.append("full.dat");
    }
    else {
        fileName.append(QString::number(number_of_solutions));
        fileName.append(".dat");
    }
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << uvWords;
    file.close();
    return true;
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
    if(unvocalizedWord.isEmpty() || unvocalizedWord.count() > 15) {
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
    }

    ///** Add word to trie **/
    //bool ret = trie->store(unvocalizedWord,1);

    if(uvWords.contains(unvocalizedWord)) {
        quint8 oldValue = uvWords.value(unvocalizedWord);
        uvWords.insert(unvocalizedWord,oldValue+1);
    }
    else {
        uvWords.insert(unvocalizedWord,1);
    }

    if(solution_counter%10000 == 0) {
           cout << solution_counter << endl;
	}

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
