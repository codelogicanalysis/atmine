#ifndef MORPHEMES_H
#define MORPHEMES_H

#include <QString>
#include <QPair>
#include <assert.h>
#include "common.h"


enum MorphemeType {Proc3, Proc2, Proc1, Proc0, Det, Prefix, Stem, Suffix, Enc0, CaseEnding, Invalid_morpheme};

const int morpheme_bits_size = 7;
extern int bit_PROC3, bit_PROC2, bit_PROC1, bit_PROC0, bit_DET, bit_ENC0, bit_CASE;
extern int *morpheme_bits[morpheme_bits_size];
static const QString morpheme_abstract_desc[morpheme_bits_size] = {"prc3", "prc2", "prc1", "prc0", "det", "enc0", "case"};
//TODO: make det
static const MorphemeType morpheme_types[morpheme_bits_size] = {Proc3, Proc2, Proc1, Proc0, Det, Enc0, CaseEnding};

class Morpheme {
    public:
        int start: 14;
        int end: 14;
        MorphemeType type: 4;
    public:
        Morpheme(int aStart = -1, int aEnd = -1, MorphemeType aType = Invalid_morpheme): start(aStart), end(aEnd),
            type(aType) {}
        item_types getItemType() const {
            switch (type) {
                case Proc3:
                case Proc2:
                case Proc1:
                case Proc0:
                case Prefix:
                    return PREFIX;

                case Stem:
                    return STEM;

                case Suffix:
                case Enc0:
                case CaseEnding:
                    return SUFFIX;

                default:
                    return ITEM_TYPES_LAST_ONE;
            }
        }
        int size() const {
            assert(end >= start - 1);
            return end - start + 1;
        }
        bool operator==(const Morpheme &m) const {
            return start == m.start && end == m.end && type == m.type;
        }
        void setType(dbitvec abstract_categories, item_types itemType) {
            typedef QPair<int, int> IndexPair;
            IndexPair range[3] = {IndexPair(0, 4), //Prefix
                                  IndexPair(0, -1), //Stem => check nothing
                                  IndexPair(5, 6)
                                 }; //Suffix
            bool typeSet = false;

            if (itemType == STEM) { //check nothing
                type = Stem;
                typeSet = true;
            } else {
                assert(itemType != ITEM_TYPES_LAST_ONE);
                IndexPair &r = range[(int)itemType];

                for (int i = r.first; i <= r.second; i++) {
                    int bit = *morpheme_bits[i];

                    if (bit >= 0 && abstract_categories.getBit(bit)) {
                        type = morpheme_types[i];
                        typeSet = true;
                    }
                }

                if (!typeSet) {
                    type = (itemType == PREFIX ? Prefix : Suffix);
                }
            }
        }
    public:
        static QString interpret(MorphemeType t) {
            static const QString morpheme_name[(int)Invalid_morpheme + 1] = {"Proc3", "Proc2", "Proc1", "Proc0", "Det", "Pref", "Stem", "Suff", "Enc0", "Case", "Invalid"};
            return morpheme_name[(int)t];
        }
};
class Morphemes  {
    private:
        QList<Morpheme> list;

        int stemIndex: 6;
        int prefixCount: 6;
        int suffixCount: 6;
    public:
        Morphemes() {
            stemIndex = -1;
            prefixCount = 0;
            suffixCount = 0;
        }
        int getStemIndex() const {
            return stemIndex;
        }
        int getAffixCount(item_types type) const {
            switch (type) {
                case PREFIX:
                    return prefixCount;

                case STEM:
                    return stemIndex >= 0;

                case SUFFIX:
                    return suffixCount;

                default:
                    return 0;
            }

            return 0;
        }
        void append(Morpheme &m) {  //assumes appended in correct order
            item_types type = m.getItemType();

            switch (type) {
                case PREFIX:
                    prefixCount++;
                    break;

                case STEM:
                    stemIndex = size();
                    break;

                case SUFFIX:
                    suffixCount++;
                    break;

                default:
                    break;
            }

            list.append(m);
        }
        bool operator ==(const Morphemes &m) const {
            return list == m.list;
        }
        Morpheme &operator [](int i) {
            return list[i];
        }
        const Morpheme &operator [](int i) const {
            return list[i];
        }
        int size() const {
            return list.size();
        }
};

//used to model information about a morpheme on which we have a diacritic
class MorphemeDiacritic {
    public:
        MorphemeType type: 5;
        int diacriticRelativePos: 10;
    public:
        MorphemeDiacritic(MorphemeType t, int pos): type(t), diacriticRelativePos(pos) {}
        bool operator ==(const MorphemeDiacritic &m) const {
            return type == m.type && diacriticRelativePos == m.diacriticRelativePos;
        }
};

//used to store unique info we are interested in
typedef QList<MorphemeDiacritic> MorphemeDiacritics;
inline unsigned int qHash(const Morpheme &m) {
    return qHash(m.start + m.end + (int)m.type);
}

inline unsigned int qHash(const Morphemes &m) {
    unsigned int h = 0;

    for (int i = 0; i < m.size(); i++) {
        h += qHash(m[i]);
    }

    return h;
}

inline unsigned int qHash(const MorphemeDiacritic &m) {
    return qHash((int)m.type + m.diacriticRelativePos);
}

//used for qHash(MorphemeDiacritics)
template<class T>
inline unsigned int qHash(const QList<T> &m) {
    unsigned int h = 0;

    for (int i = 0; i < m.size(); i++) {
        h += qHash(m[i]);
    }

    return h;
}



void morpheme_initialize();


#endif
