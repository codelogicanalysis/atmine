#include "sarftag.h"
#include "global.h"
#include "amtmainwindow.h"

class AMTMainWindow;

SarfTag::SarfTag(int start, int length, QString *text, QWidget *parent): Stemmer(text,0)
{
    this->text = *text;
    this->start = start;
    this->length = length;
}

bool SarfTag::on_match() {

    for( int i=0; i< (_atagger->tagTypeVector->count()); i++) {

        QString _tag = _atagger->tagTypeVector->at(i)->tag;
        bool belong = false;

        /** Check if tag source is sarf tag types **/
        if(_atagger->tagTypeVector->at(i)->source != sarf) {
            continue;
        }

        const SarfTagType * tagtype = (SarfTagType*)(_atagger->tagTypeVector->at(i));
        for(int j=0; j < (tagtype->tags.count()); j++) {

            bool contain = false;
            const Quadruple< QString , QString , QString , QString > * tag = &(tagtype->tags.at(j));
            if(tag->first == "Prefix") {
                for(int k=0;k<prefix_infos->size();k++) {

                    minimal_item_info & pre = (*prefix_infos)[k];
                    if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
                            continue;

                    if(pre.raw_data == tag->second) {
                        contain = true;
                        break;
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Stem") {

                minimal_item_info & stem = *stem_info;
                if(stem.raw_data == tag->second) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Category") {

                minimal_item_info & stem = *stem_info;
                int id = tag->second.toInt();

                for(unsigned int i=0; i< stem.abstract_categories.length(); i++) {
                    if (stem.abstract_categories[i]) {
                        int abstract_id=database_info.comp_rules->getAbstractCategoryID(i);
                        if(abstract_id == id) {
                            contain = true;
                            break;
                        }
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
            }
            else if(tag->first == "Suffix") {
                for(int k=0;k<suffix_infos->size();k++) {

                    minimal_item_info & suff = (*suffix_infos)[k];
                    if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                            continue;

                    if(suff.raw_data == tag->second) {
                        contain = true;
                        break;
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Stem-POS") {

                minimal_item_info & stem = *stem_info;
                if(stem.POS.contains(tag->second,Qt::CaseSensitive)) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Prefix-POS") {

                for(int k=0;k<prefix_infos->size();k++) {

                    minimal_item_info & pre = (*prefix_infos)[k];
                    if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
                            continue;

                    if(pre.POS == tag->second) {
                        contain = true;
                        break;
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Suffix-POS") {

                for(int k=0;k<suffix_infos->size();k++) {

                    minimal_item_info & suff = (*suffix_infos)[k];
                    if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                            continue;

                    if(suff.POS == tag->second) {
                        contain = true;
                        break;
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
            else if(tag->first == "Gloss") {

                for(int k=0;k<prefix_infos->size();k++) {

                    minimal_item_info & pre = (*prefix_infos)[k];
                    if (pre.POS.isEmpty() && pre.raw_data.isEmpty())
                            continue;

                    if(pre.description().split('/').contains(tag->second)) {
                        contain = true;
                        break;
                    }
                }

                if(contain) {
                   belong = true;
                   break;
                }

                minimal_item_info & stem = *stem_info;
                if(stem.description().split('/').contains(tag->second)) {
                    belong = true;
                    break;
                }

                for(int k=0;k<suffix_infos->size();k++) {

                    minimal_item_info & suff = (*suffix_infos)[k];
                    if (suff.POS.isEmpty() && suff.raw_data.isEmpty())
                            continue;

                    if(suff.description().split('/').contains(tag->second)) {
                        contain = true;
                        break;
                    }
                }
                if(contain) {
                    belong = true;
                    break;
                }
                else {
                    continue;
                }
            }
        }

        if(belong) {
            // Add tag with this tagtype
            bool add = true;
            for( int z=0; z<(_atagger->tagVector.count()); z++) {
                const Tag * tag2 = &(_atagger->tagVector.at(z));
                if((tag2->pos == start) && (tag2->length == length) && (tag2->type == _tag)) {
                    add = false;
                    break;
                }
            }
            if(add) {
                _atagger->insertTag(_tag,start,length,sarf,original);
            }
        }
    }
    return true;
}
