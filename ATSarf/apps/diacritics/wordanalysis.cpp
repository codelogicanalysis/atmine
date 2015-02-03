#include "wordanalysis.h"

WordAnalysis::WordAnalysis(QString *text, int *count) : Stemmer(text,0)
{
    this->text = text;
    this->count = count;
    *count = 0;
}

bool WordAnalysis::on_match()
{
    *count = *count + 1;
    Solution sol;
    sol.isValid = true;
    if( stem_info->POS.isEmpty() || stem_info->POS.contains('+')) {
        sol.isValid = false;
    }

    QString vocalizedWord;
    QString unvocalizedWord;
    int prefix_infos_size = prefix_infos->size();
    for (int i= 0; i<prefix_infos_size;i++) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(!(pre.raw_data.isEmpty())) {
            vocalizedWord.append(pre.raw_data);
        }
    }

    vocalizedWord.append(stem_info->raw_data);
    int stem_length = removeDiacritics(stem_info->raw_data).count();

    int suffix_infos_size = suffix_infos->size();
    for (int i=0;i<suffix_infos_size;i++) {
        minimal_item_info & suff = (*suffix_infos)[i];
        if(!(suff.raw_data.isEmpty())) {
            vocalizedWord.append(suff.raw_data);
        }
    }

    unvocalizedWord = removeDiacritics(vocalizedWord);
    sol.length = stem_length;
    if(!(stem_info->POS.isEmpty())) {
        sol.stemPOS = stem_info->POS.split('/').at(1);
    }
    sol.stem = vocalizedWord;//stem_info->raw_data;
    if(stem_length < unvocalizedWord.count()) {
        sol.isValid = false;
    }

    solutions.append(sol);

    return true;
}

int WordAnalysis::getAmbiguity() {
    return (*count);
}
