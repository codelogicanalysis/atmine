/**
  * @file   common.h
  * @author Jad Makhlouta and documented by Ameen Jaber
  * @brief  This file implements common structures and enums used by the different classes and routines defined
  */
#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>
#include "dbitvec.h"

//#define USE_ORIGINAL
#define USE_BAMA

#define MULTIPLICATION

#define TIMESTAMP

#define RUNON_WORDS
#define ENABLE_RUNON_WORD_INSIDE_COMPOUND_WORD
#define REMOVE_ONE_LETTER_ABBREVIATIONS_FROM_BEING_IN_RUNONWORDS
#define ALLOW_MULTIPLE_RESULTING_CATEGORIES

using namespace std;

#define max_sources 256

extern bool filling;

extern dbitvec INVALID_BITSET;

void initialize_variables(); //must be called at the start - called by initialize
void initialize_other(); //must be called at start but after datastructures loaded

/**
  * @enum   rules
  * @brief  This enumerator defines the different rule classes for the matching different solutions
  */
enum rules { AA, AB, AC, BC, CC, RULES_LAST_ONE };

/**
  * @enum   item_types
  * @brief  This enumerator defines the different types of an item being prefix,suffix, or stem
  */
enum  item_types { PREFIX, STEM, SUFFIX, ITEM_TYPES_LAST_ONE};   // Ask about the last one

/**
  * @enum   enumeration_type
  * @brief  This enumerator defines the different types of solutions that the enumerator can generate
  */
enum  enumeration_type { ENUMALL, ENUMSTEM};   // Ask about the last one

/**
  * @typedef minimal_item_info
  * @brief The following defines a class type including the minimal information of a match which could be stem or an affix
  */
typedef class minimal_item_info_ {
    private:
        /// This includes the description of a matching prefix/stem/suffix which is the gloss
        QString desc;
        long desc_id;
    public:
        /// The type of the item being prefix, suffix, or stem
        item_types type;
        long category_id;
        dbitvec abstract_categories; //only for STEMS
        /// This string holds the root or stem of the input string
        QString raw_data;
        /// This string saves the part of speech of the match
        QString POS;
    public:

        void setDescription(long desc_id) {
            desc = "";
            this->desc_id = desc_id;
        }
        void setDescription(QString desc) {
            this->desc = desc;
            desc_id = -1;
        }
        long description_id() const {
            return desc_id;
        }
        QString description();

        minimal_item_info_() {
            abstract_categories.resize(max_sources);
        }
} minimal_item_info;

typedef class all_item_info_ : public minimal_item_info_ {
    public:
        unsigned long long item_id;
        dbitvec sources;
        QString lemma_ID; //only for STEMs

        all_item_info_(): minimal_item_info_() {
            sources.resize(max_sources);
        }
} all_item_info;

/**
  * @typedef    text_info
  * @brief  The following defines a structure for the text information which is passed to different parts of the stemmer.
  * It includes a pointer to the text and start/finish indixes
  */
typedef struct text_info_ {
    QString *text;
    long start, finish;
    QString getString() {
        //TODO: solve bug in inconsistent start when diacritics exist
        unsigned int len = finish - start + 1;
        return text->mid(start, len);
    }
} text_info;

/**
  * @class  multiply_params
  * @brief  This class defines the different parameter settings for the solution extracted. Based on the required
  * details of the solution required, the booleans within are set.
  */
class multiply_params {
    public:
        bool raw_data: 1;
        bool description: 1;
        bool POS: 1;
        bool abstract_category: 1;

        void setAll() {
            raw_data = true;
            description = true;
            POS = true;
            abstract_category = true;
        }
        bool raw_dataONLY() {
            return (raw_data && !description && !POS && !abstract_category);
        }
        bool NONE() {
            return (!raw_data && !description && !POS && !abstract_category);
        }
        bool ALL() {
            return (raw_data && description && POS && abstract_category);
        }

        /**
          * This method is the constructor of the class multiply params
          */
        multiply_params() {
            setAll();
        }
};

static multiply_params M_ALL;

#ifndef SUBMISSION
static const QString databaseFileName = "../../src/sql design/atm_filled.sql";
#else
static const  QString databaseFileName = ".atm_filled.sql";
#endif

    #ifdef USE_ORIGINAL
        static const QString tag = "_original";
    #elif defined(USE_BAMA)
        static const QString tag = "_bama";
    #else
        static const QString tag = "";
    #endif
    static const QString trie_path = ".stem_trie" + tag + ".dat";
    static const QString trie_list_path = ".stem_list" + tag + ".dat";
    static const QString compatibility_rules_path = ".compatibility" + tag + ".dat";
    static const QString prefix_tree_path = ".prefix_tree" + tag + ".dat";
    static const QString suffix_tree_path = ".suffix_tree" + tag + ".dat";
    static const QString description_path = ".descriptions" + tag + ".dat";
    static const QString prefix_info_path = ".prefix_info" + tag + ".dat";
    static const QString suffix_info_path = ".suffix_info" + tag + ".dat";
    static const QString stem_info_path = ".stem_info" + tag + ".dat";

//reverse_descriotion
#define REVERSEDESCBIT max_sources-2
#define setReverseDirection(abstractCat) abstractCat.setBit(REVERSEDESCBIT,true)
#define resetReverseDirection(abstractCat) abstractCat.setBit(REVERSEDESCBIT,false)
#define isReverseDirection(abstractCat) abstractCat.getBit(REVERSEDESCBIT)

#define absVal(v) (v>=0?v:-v)

#endif


