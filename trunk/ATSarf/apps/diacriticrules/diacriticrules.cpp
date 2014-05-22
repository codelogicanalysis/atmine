#include <QStringList>
#include "diacriticrules.h"
#include "wordambiguity.h"

DiacriticRules::DiacriticRules(long number_of_solutions, bool get_all_details) : Enumerator(get_all_details)
{
    solution_counter = 1;
    filtered_items = 0;
    this->number_of_solutions = number_of_solutions;
};

bool DiacriticRules::on_match()
{
    double ambiguity_reduction = 0.0;
    int least_ambiguity_position = -1;

    /** Number of Morphemes **/
    int number_of_morphemes = 0;

    /** letter count of unvocalized word **/
    int length = 0;

    QString vocalizedWord;
    QString unvocalizedWord;
    QVector<QString> prefixPOSs;
    QVector<QString> prefixes;
    QString stemPOS;
    QVector<QString> suffixPOSs;
    QVector<QString> suffixes;

    int prefix_length = 0;
    /** letter count of stem **/
    int stem_length = 0;
    int suffix_length = 0;

    /** Get vocalized and unvocalized words **/
    int prefix_infos_size = prefix_infos->size();
    for (int i= 0; i<prefix_infos_size;i++) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(!(pre.raw_data.isEmpty())) {
            number_of_morphemes++;
            vocalizedWord.append(pre.raw_data);
        }
    }
    prefix_length = removeDiacritics(vocalizedWord).count();

    number_of_morphemes++;
    vocalizedWord.append(stem_info->raw_data);
    stem_length = removeDiacritics(stem_info->raw_data).count();

    int suffix_infos_size = suffix_infos->size();
    for (int i=0;i<suffix_infos_size;i++) {
        minimal_item_info & suff = (*suffix_infos)[i];
        if(!(suff.raw_data.isEmpty())) {
            number_of_morphemes++;
            vocalizedWord.append(suff.raw_data);
        }
    }

    unvocalizedWord = removeDiacritics(vocalizedWord);

    /** Unvocalized word Character Count **/
    length = unvocalizedWord.count();
    suffix_length = length - (prefix_length + stem_length);

    /** Ambiguity of the unvocalized word **/
    int unvocalizedAmbiguity = 0;
    WordAmbiguity wa(&unvocalizedWord, &unvocalizedAmbiguity);
    wa();

    /** Discard this morphological solution if the unvocalized word is not ambiguous (has 1 morpho. solution) **/
    if(unvocalizedAmbiguity < 2) {
        return true;
    }

    /// Select required morphological features

    /** Prefix Features **/
    int j = 0;
    for (int i = (prefix_infos_size-1); (i>=0) && (j<4);i--) {
        minimal_item_info & pre = (*prefix_infos)[i];
        if(pre.POS.isEmpty() && pre.raw_data.isEmpty()) {
            continue;
        }

        QStringList pre_poss = pre.POS.split('/');
        if(pre_poss.count() != 2) {
            continue;
        }
        QString unvoc_pre_data = removeDiacritics(pre.raw_data);
        if(!(unvoc_pre_data.isEmpty())) {
            prefixes.prepend(unvoc_pre_data);
        }
        if(!(pre_poss[1].isEmpty())) {
            prefixPOSs.prepend(pre_poss[1]);
        }
        j++;
    }

    while(prefixes.count() < 4) {
        prefixes.prepend("EPRE");
    }

    while(prefixPOSs.count() < 4) {
        prefixPOSs.prepend("EPREPOS");
    }

    /** Stem Features **/
    minimal_item_info & stem = *stem_info;
    //stem_length = removeDiacritics(stem.raw_data).count();
    QStringList stem_poss = stem.POS.split('/');
    if(stem_poss.count() != 2) {
        return true;
    }
    stemPOS = stem_poss[1];

    /** Suffix Features **/
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
            suffixes.append(unvoc_suf_data);
        }
        if(!(suff_poss[1].isEmpty())) {
            suffixPOSs.append(suff_poss[1]);
        }
        j++;
    }

    while(suffixes.count() < 4) {
        suffixes.append("ESUF");
    }

    while(suffixPOSs.count() < 4) {
        suffixPOSs.append("ESUFPOS");
    }

    /// Detach diacritics from raw_data and store in separate structure
    int diacritic_Counter = 0;
    QVector<QVector<QChar> > wordDiacritics(length);
    int letterIndex = 0;
    for(int i=1; i<vocalizedWord.count(); i++) {
        QChar currentLetter= vocalizedWord[i];
        if(isDiacritic(currentLetter)) {
            wordDiacritics[letterIndex].append(currentLetter);
            diacritic_Counter++;
        }
        else {
            letterIndex++;
        }
    }

    if(diacritic_Counter == 0) {
        return true;
    }

    /// Get the number of solutions for each solution with one diacritic
    /// Select diacritic position leastambiguous = least number of morphological solutions
    QVector<QVector<int> > diacriticAmbiguity(length);
    int least_ambiguity = unvocalizedAmbiguity + 1;
    int diacritic_Index = -1;
    for(int i=0; i< wordDiacritics.count(); i++) {
        for(j=0; j< wordDiacritics.at(i).count(); j++) {
            QString one_diacritic_word = unvocalizedWord;
            one_diacritic_word.insert(i+1,wordDiacritics[i][j]);

            int one_diacritic_Ambiguity = 0;
            WordAmbiguity wa(&one_diacritic_word, &one_diacritic_Ambiguity);
            wa();

            if(one_diacritic_Ambiguity == 0) {
                diacriticAmbiguity[i].append(unvocalizedAmbiguity);
            }
            else {
                diacriticAmbiguity[i].append(one_diacritic_Ambiguity);
            }

            if(diacriticAmbiguity[i][j] <  least_ambiguity) {
                least_ambiguity = diacriticAmbiguity[i][j];
                least_ambiguity_position = i;
                diacritic_Index = j;
            }
        }
    }

    /** This weirdly happens when a word partial diacritics has ambiguity more than the unvocalized word (ex. dAn) **/
    if((least_ambiguity_position == -1) || (diacritic_Index == -1)) {
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

    ambiguity_reduction = ((unvocalizedAmbiguity- diacriticAmbiguity[least_ambiguity_position][diacritic_Index]) * 1.0) / unvocalizedAmbiguity;

    /** Filter data to extract high ambiguity reduction instances **/

    if(ambiguity_reduction < 0.667) {
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
    filtered_items++;

    /** Print/Use data **/

    theSarf->out << number_of_morphemes << "  "
            << length << "  "
            << stem_length << "  ";

    //    prefixPOSs
    for(int i=0; i<prefixPOSs.count(); i++) {
        theSarf->out << prefixPOSs[i] << "  ";
    }

    //    prefixes
    for(int i=0; i< prefixes.count(); i++) {
        theSarf->out << prefixes[i] << "  ";
    }

    //    stemPOS
    theSarf->out << stemPOS << "  ";

    //    suffixPOSs
    for(int i=0; i<suffixPOSs.count(); i++) {
        theSarf->out << suffixPOSs[i] << "  ";
    }

    //    suffixes
    for(int i=0; i<suffixes.count(); i++) {
        theSarf->out << suffixes[i] << "  ";
    }

    //    least_ambiguity_position
    //    prefixs , prefixm, prefixe , stems , stemm, steme , suffixs , suffixm, suffixe
    //theSarf->out << least_ambiguity_position << "  ";
    QString diacritic_position;

    if((prefix_length != 0) && (least_ambiguity_position == 0)) {
        diacritic_position = "prefixs";
    }
    else if((prefix_length != 0) && (least_ambiguity_position > 0) && (least_ambiguity_position < (prefix_length-1))) {
        diacritic_position = "prefixm";
    }
    else if((prefix_length != 0) && (least_ambiguity_position == (prefix_length-1))) {
        diacritic_position = "prefixe";
    }
    else if(least_ambiguity_position == prefix_length) {
        diacritic_position = "stems";
    }
    else if((least_ambiguity_position > (prefix_length)) && (least_ambiguity_position < (prefix_length + stem_length - 1))) {
        diacritic_position = "stemm";
    }
    else if(least_ambiguity_position == (prefix_length + stem_length - 1)) {
        diacritic_position = "steme";
    }
    else if((suffix_length != 0) && (least_ambiguity_position == (prefix_length + stem_length))) {
        diacritic_position = "suffixs";
    }
    else if((suffix_length != 0) && (least_ambiguity_position > (prefix_length + stem_length)) && (least_ambiguity_position < (length - 1))) {
        diacritic_position = "suffixm";
    }
    else if((suffix_length != 0) && (least_ambiguity_position == (length -1))) {
        diacritic_position = "suffixe";
    }
    else {
        cout << "Couldn't set diacritic position!" << endl;
        return false;
    }
    theSarf->out << diacritic_position << '\n';

    //    ambiguity_reduction
    //theSarf->out << ambiguity_reduction << '\n';

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
