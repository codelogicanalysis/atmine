#include "number.h"
#include <getGloss.h>

Number::Number(NumNorm *controller, QString word) : Stemmer(&word, 0)
{
    this->controller = controller;
    hashGlossInt = &(controller->hashGlossInt);
    this->word = &word;
    controller->isNumberDone = true;
}

bool Number::on_match() {
    minimal_item_info & stem = *stem_info;

    int target_category_id = database_info.comp_rules->getAbstractCategoryID("Number");
    for(unsigned int k=0; k< stem.abstract_categories.length(); k++) {
        if (stem.abstract_categories[k]) {
            int category_id = database_info.comp_rules->getAbstractCategoryID(k);
            if(target_category_id == category_id) {

                controller->isNumberDone = false;
                minimal_item_info & stem = *stem_info;
                QStringList stem_glosses = getGloss(stem.description());

                int val = -1;
                if(isDigitsTens(stem_glosses, val)) {
                    digitsTensActions(val);
                }
                else if(isKey(stem_glosses, val)) {
                    keyActions(val);
                }
                else if(isHundred(stem_glosses, val)) {
                    hundredActions(val);
                }
                return false;
            }
        }
    }
    return true;
}

bool Number::isDigitsTens(QStringList& stem_glosses, int& val) {
    for(int i=0; i<stem_glosses.count(); i++) {
        QHash<QString, int>::const_iterator it = hashGlossInt->find(stem_glosses[i]);
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

bool Number::isKey(QStringList& stem_glosses, int& val) {
    for(int i=0; i<stem_glosses.count(); i++) {
        QHash<QString, int>::const_iterator it = hashGlossInt->find(stem_glosses[i]);
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

bool Number::isHundred(QStringList& stem_glosses, int& val) {
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

void Number::digitsTensActions(int val) {
    if(controller->isHundred) {
                            controller->currentH += val;
                    }
                    else {
                            if(controller->current == 0) {
                                    controller->current = val;
                            }
                            else {
                                    if(controller->isKey) {
                                            controller->previous += controller->current;
                                            controller->current = val;
                                    }
                                    else {
                                            controller->current += val;
                                    }
                            }
                    }
                    controller->isKey = false;
}

void Number::keyActions(int val) {
    if(controller->isHundred) {
        if(controller->current != 0) {
            controller->previous += controller->current;
            //cout << "current " << controller->current << " previous " << controller->previous << '\n';
        }
        controller->current = controller->currentH * val;
        controller->currentH = 0;
        controller->isHundred = false;
        controller->isKey = true;
    }
    else {
        if(controller->current == 0) {
            controller->current = val;
            controller->isKey = true;
        }
        else {
            if(!(controller->isKey)) {
                controller->isKey = true;
                controller->current = controller->current * val;
            }
            else {
                controller->previous += controller->current;
                controller->current = val;
            }
        }
    }
}

void Number::hundredActions(int val) {
    controller->isHundred = true;
    if(controller->current == 0) {
        controller->currentH = val;
    }
    else {
        if(!(controller->isKey)) {
            controller->currentH = controller->current * val;
            controller->current = 0;
        }
        else {
            controller->currentH = val;
        }
    }
    controller->isKey = false;
}
