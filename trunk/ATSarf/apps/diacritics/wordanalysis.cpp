#include "wordanalysis.h"

WordAnalysis::WordAnalysis(QString *text, int *count, QString morpheme_type) : Stemmer(text,0)
{
    this->text = text;
    this->count = count;
    *count = 0;
    this->full = (morpheme_type == "A");
}

bool WordAnalysis::on_match()
{
    *count = *count + 1;
    Solution sol;
    if( stem_info->POS.isEmpty() || stem_info->POS.contains('+')) {
        sol.isValid = false;
    }

    /** vocalized word and morpheme features **/
    QString vocalizedWord;
    QString unvocalizedWord;
    int prefix_infos_size = prefix_infos->size();
    for (int i= 0; i<prefix_infos_size;i++) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(!(pre.raw_data.isEmpty())) {
            sol.number_of_morphemes = sol.number_of_morphemes + 1;
            vocalizedWord.append(pre.raw_data);
        }
    }
    sol.prefix_length = removeDiacritics(vocalizedWord).count();

    sol.vStem = stem_info->raw_data;
    vocalizedWord.append(stem_info->raw_data);
    sol.stem_length = removeDiacritics(stem_info->raw_data).count();
    sol.number_of_morphemes = sol.number_of_morphemes + 1;

    int suffix_infos_size = suffix_infos->size();
    for (int i=0;i<suffix_infos_size;i++) {
        minimal_item_info & suff = (*suffix_infos)[i];
        if(!(suff.raw_data.isEmpty())) {
            sol.number_of_morphemes = sol.number_of_morphemes + 1;
            vocalizedWord.append(suff.raw_data);
        }
    }
    unvocalizedWord = removeDiacritics(vocalizedWord);
    sol.length = unvocalizedWord.count();
    sol.suffix_length = sol.length - (sol.prefix_length + sol.stem_length);

    /** extract POS and raw data info **/

    // prefixes
    int j = 0;
    for (int i = (prefix_infos_size-1); (i>=0) && (j<4);i--) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(pre.POS.isEmpty()) {
            continue;
        }

        QStringList pre_poss = pre.POS.split('/');
        if(pre_poss.count() != 2) {
            continue;
        }
        QString unvoc_pre_data = removeDiacritics(pre.raw_data);
        if(!(unvoc_pre_data.isEmpty())) {
            sol.prefixes.prepend(unvoc_pre_data);
        }
        if(!(pre_poss[1].isEmpty())) {
            sol.prefixPOSs.prepend(pre_poss[1]);
        }
        j++;
    }

    while(sol.prefixes.count() < 4) {
        sol.prefixes.prepend("EPRE");
    }

    while(sol.prefixPOSs.count() < 4) {
        sol.prefixPOSs.prepend("EPREPOS");
    }

    // stem
    if(!(stem_info->POS.isEmpty())) {
        sol.stemPOS = stem_info->POS.split('/').at(1);
    }

    // suffixes
    j = 0;
    for (int i=0;(i<suffix_infos_size) && (j<4);i++) {
        minimal_item_info & suff = (*suffix_infos)[i];
        if(suff.POS.isEmpty() && suff.raw_data.isEmpty()) {
            continue;
        }

        QStringList suff_poss = suff.POS.split('/');
        if(suff_poss.count() != 2) {
            continue;
        }
        QString unvoc_suf_data = removeDiacritics(suff.raw_data);
        if(!(unvoc_suf_data.isEmpty())) {
            sol.suffixes.append(unvoc_suf_data);
        }
        if(!(suff_poss[1].isEmpty())) {
            sol.suffixPOSs.append(suff_poss[1]);
        }
        j++;
    }

    while(sol.suffixes.count() < 4) {
        sol.suffixes.append("ESUF");
    }

    while(sol.suffixPOSs.count() < 4) {
        sol.suffixPOSs.append("ESUFPOS");
    }

    // remove tanween since it is useless in problem at hand
    QChar last = vocalizedWord[vocalizedWord.count()-1];
    if(last == fathatayn || last == dammatayn || last == kasratayn) {
        vocalizedWord.remove(vocalizedWord.count()-1,1);
    }

    sol.vWord = vocalizedWord;

    if(!full) {
        if(sol.stem_length != sol.length) {
            sol.isValid = false;
        }
    }

    solutions.append(sol);

    return true;
}

int WordAnalysis::getAmbiguity() {
    return (*count);
}
