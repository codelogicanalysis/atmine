#include "autotagger.h"
#include "global.h"

AutoTagger::AutoTagger(QString *text, QHash<QString, QSet<QString> > *synSetHash) {
    this->text = text;
    this->synSetHash = synSetHash;
}

bool AutoTagger::operator ()() {
    int start = 0;
    while(start != text->count()) {
        Word word = nextWord(*text, start);
        if(word.word.isEmpty()) {
            break;
        }
        int length = word.end - word.start + 1;
        //excludedNegationFormula
        QSet<QString> eNF;
        //includedNegationFormula
        QHash<QString, QString> iNF;

        SarfTag sarftag(word.start, length, &(word.word), synSetHash, &eNF, &iNF);
        sarftag();

        /** Clean tags to fix the negation **/

        QHashIterator<QString,QString> iNFIterator(iNF);
        while(iNFIterator.hasNext()) {
            iNFIterator.next();
            QString tag = iNFIterator.value();
            bool add = true;
            for( int z=0; z<(_atagger->tagVector.count()); z++) {
                const Tag * tag2 = &(_atagger->tagVector.at(z));
                if((tag2->pos == word.start) && (tag2->length == length) && (tag2->type == tag)) {
                    add = false;
                    break;
                }
            }
            if(add) {
                _atagger->insertTag(tag,word.start,length,sarf,original);
            }
        }
        /** End of cleaning **/

        start = word.end + 1;
    }

    return true;
}
