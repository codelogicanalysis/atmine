#include <QStringList>
#include "worddensity.h"

WordDensity::WordDensity(long max_word_length, bool get_all_details) : Enumerator(get_all_details)
{
    this->max_word_length = max_word_length;
    number_of_uvWords = new long long[max_word_length];
    number_of_vWords = new long long[max_word_length];
    for(int i=0; i< max_word_length; i++) {
        number_of_uvWords[i] = 0;
    }
    for(int i=0; i< max_word_length; i++) {
        number_of_vWords[i] = 0;
    }
};

bool WordDensity::on_match()
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
    int wordLength = unvocalizedWord.count();
    if(wordLength > max_word_length) {
        return true;
    }

    if(!(wordSet.contains(unvocalizedWord))) {
        wordSet.insert(unvocalizedWord);
        number_of_uvWords[wordLength-1] = number_of_uvWords[wordLength-1] + 1;
    }
    number_of_vWords[wordLength-1] = number_of_vWords[wordLength-1] + 1;

    return true;
};

void WordDensity::getuvWordStats() {
    theSarf->out << "The statistics of the unvocalized Words in Arabic of max length " << max_word_length << " are:" << endl;
    for(int i=0; i < max_word_length; i++) {
    	theSarf->out << number_of_uvWords[i] << endl;
    }
}

void WordDensity::getvWordStats() {
    theSarf->out << "The statistics of the vocalized Words in Arabic of max length " << max_word_length << " are:" << endl;
    for(int i=0; i < max_word_length; i++) {
        theSarf->out << number_of_vWords[i] << endl;
    }
}
