#include "amfiller.h"

AMFiller::AMFiller(QString text, QString *sarfMatches, QString mVName)
    : Stemmer(&text,0)
{
    this->text = text;
    this->sarfMatches = sarfMatches;
    this->mVName = mVName;
    sarfMatches->append("vector<Match> " + mVName + ";\n");
    this->count = 0;
}

bool AMFiller::on_match() {

    QString mName = "_match" + QString::number(count);
    sarfMatches->append("Match " + mName + ";\n");

    /** Getting stem features **/
    sarfMatches->append(mName + ".stem = \"" + stem_info->raw_data + "\";\n");
    sarfMatches->append(mName + ".stemPOS = \"" + stem_info->POS + "\";\n");
    sarfMatches->append(mName + ".stemGloss = \"" + stem_info->description() + "\";\n");

    /** Getting prefixes features **/
    for(int i=0;i<prefix_infos->size();i++) {

        minimal_item_info & pre = (*prefix_infos)[i];
        if (pre.POS.isEmpty() && pre.raw_data.isEmpty()) {
            continue;
        }

        sarfMatches->append(mName + ".prefix[" + QString::number(i) + "] = \"" + pre.raw_data + "\";\n");
        sarfMatches->append(mName + ".prefixPOS[" + QString::number(i) + "] = \"" + pre.POS + "\";\n");
        sarfMatches->append(mName + ".prefixGloss[" + QString::number(i) + "] = \"" + pre.description() + "\";\n");
    }

    /** Getting suffixes features **/
    for(int i=0;i<suffix_infos->size();i++) {

        minimal_item_info & suff = (*suffix_infos)[i];
        if (suff.POS.isEmpty() && suff.raw_data.isEmpty()) {
            continue;
        }

        sarfMatches->append(mName + ".suffix[" + QString::number(i) + "] = \"" + suff.raw_data + "\";\n");
        sarfMatches->append(mName + ".suffixPOS[" + QString::number(i) + "] = \"" + suff.POS + "\";\n");
        sarfMatches->append(mName + ".suffixGloss[" + QString::number(i) + "] = \"" + suff.description() + "\";\n");
    }

    /** Add match solution to vector **/
    sarfMatches->append(mVName + ".push_back(" + mName + ");\n");

    return true;
}
