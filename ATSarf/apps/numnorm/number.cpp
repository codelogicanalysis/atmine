#include "number.h"
#include <getGloss.h>

Number::Number(NumNorm *_controller, Word* _word, long * _val) : Stemmer(&(_word->word), 0)
{
    val = _val;
    *val = -1;
    controller = _controller;
    hashGlossInt = &(_controller->hashGlossInt);
    word = &(_word->word);
    start = _word->start;
    end = _word->end;
    _controller->isNumberDone = true;
    _controller->numtype = None;
    numstate = Nothing;
}

bool Number::on_match() {
    minimal_item_info & stem = *stem_info;

    QStringList glosses = getGloss(stem.description());


    //int target_category_id = database_info.comp_rules->getAbstractCategoryID("Number");
    //for(unsigned int c=0; c< stem.abstract_categories.length(); c++) {
    const int NUMBER_CATEGORY_INDEX = 142;

    if (stem.abstract_categories[NUMBER_CATEGORY_INDEX] || glosses.contains("ninety") || glosses.contains("one")) {
        //int category_id = database_info.comp_rules->getAbstractCategoryID(c);
        //if(target_category_id == category_id) {

            controller->isNumberDone = false;
            minimal_item_info & stem = *stem_info;
            QStringList stem_glosses = getGloss(stem.description());

            if(isDigitsTens(stem_glosses, *val)) {
                controller->numtype = TenDigit;
                numstate = Done;
                if(controller->numberStart == -1) {
                    controller->numberStart = start;
                    controller->numberEnd = end;
                }
                else {
                    controller->numberEnd = end;
                }
            }
            else if(isKey(stem_glosses, *val)) {
                numstate = Continue;
                for(int k=0;k<suffix_infos->size();k++) {

                    minimal_item_info & suff = (*suffix_infos)[k];
                    if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                        continue;

                    QStringList suff_glosses = getGloss(suff.description());
                    if(suff_glosses.contains("two")) {
                        (*val) = (*val) * 2;
                        numstate = Done;
                        break;
                    }

                }
                controller->numtype = Key;
                if(controller->numberStart == -1) {
                    controller->numberStart = start;
                    controller->numberEnd = end;
                }
                else {
                    controller->numberEnd = end;
                }
            }
            else if(isHundred(stem_glosses, *val)) {
                numstate = Continue;
                for(int k=0;k<suffix_infos->size();k++) {

                    minimal_item_info & suff = (*suffix_infos)[k];
                    if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                        continue;

                    QStringList suff_glosses = getGloss(suff.description());
                    if(suff_glosses.contains("two")) {
                        (*val) = (*val) * 2;
                        numstate = Done;
                        break;
                    }

                }
                controller->numtype = Hundred;
                if(controller->numberStart == -1) {
                    controller->numberStart = start;
                    controller->numberEnd = end;
                }
                else {
                    controller->numberEnd = end;
                }
            }
            if(numstate == Done) {
                return false;
            }
            else {
                return true;
            }
        //}
    }
    //}
    return true;
}

bool Number::isDigitsTens(QStringList& stem_glosses, long& val) {
    for(int i=0; i<stem_glosses.count(); i++) {
        QHash<QString, long>::const_iterator it = hashGlossInt->find(stem_glosses[i]);
        if(it == hashGlossInt->end()) {
            continue;
        }
        else {
            if(it.value() < 100 && it.value() > 0) {
                val = it.value();
                return true;
            }
        }
    }
    val = -1;
    return false;
}

bool Number::isKey(QStringList& stem_glosses, long& val) {
    for(int i=0; i<stem_glosses.count(); i++) {
        QHash<QString, long>::const_iterator it = hashGlossInt->find(stem_glosses[i]);
        if(it == hashGlossInt->end()) {
            continue;
        }
        else {
            if(it.value() == 1000 || it.value() == 1000000 || it.value() == 1000000000) {
                val = it.value();
                return true;
            }
        }
    }
    val = -1;
    return false;
}

bool Number::isHundred(QStringList& stem_glosses, long& val) {
    int index = stem_glosses.indexOf("hundred");
    if(index >=0 ) {
        val = hashGlossInt->value("hundred");
        return true;
    }
    else {
        val = -1;
        return false;
    }
}
