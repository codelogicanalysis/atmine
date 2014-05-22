/**
  * @file   enumerator.h
  * @brief  this header file contains the definition of the enumerator class which enumerates all the Arabic words
  * @author Ameen Jaber
  */

#ifndef _ENUMERATOR_H
#define	_ENUMERATOR_H

#include "prefix_enumerate.h"
#include "stem_enumerate.h"
#include "suffix_enumerate.h"
#include "stemmer.h"
#include "common.h"
#include <QVector>

//class SarfParameters {
//public:
//        /// Boolean in order to enable/disable run on words
//        bool enableRunonwords;

//        /**
//          * This is the constructor of the SarfParameters class which by default sets the run on words capability to false
//          */
//        SarfParameters() {
//                enableRunonwords=false;
//        }
//};
//extern SarfParameters sarfParameters;

class Enumerator;

class PrefixEnumeratorMachine: public PrefixEnumerate
{
public:
        Enumerator * controller;

        PrefixEnumeratorMachine(Enumerator * controller);
        bool onMatch();
        virtual ~PrefixEnumeratorMachine(){}
};

class StemEnumeratorMachine: public StemEnumerate
{
public:
        Enumerator * controller;

        StemEnumeratorMachine(Enumerator * controller, long prefix_category);
        bool onMatch();
        virtual ~StemEnumeratorMachine(){}
};

class SuffixEnumeratorMachine: public SuffixEnumerate
{
public:
        Enumerator * controller;

        SuffixEnumeratorMachine(Enumerator * controller, long prefix_category, long stem_category);
        bool onMatch();
        virtual ~SuffixEnumeratorMachine(){}
};

class Enumerator {
public:

        multiply_params multi_p;
        item_types type;
        bool get_all_details;
        QVector<minimal_item_info> * prefix_infos;
        minimal_item_info * stem_info;
        QVector<minimal_item_info> * suffix_infos;

    public:
        bool called_everything;

        PrefixEnumeratorMachine* Prefix;

        StemEnumeratorMachine* Stem;

        SuffixEnumeratorMachine* Suffix;

        Enumerator(bool get_all_details=true) {
                // Initialize the prefix,stem, and suffix info to null
                prefix_infos=NULL;
                stem_info=NULL;
                suffix_infos=NULL;
                this->get_all_details=get_all_details;  // checking from input whether to get all details of word morphology
                Prefix=new PrefixEnumeratorMachine(this);   // Initializing the Prefix machine passing start index and pointer to current class
                Stem=NULL;  // Set Stem machine to NULL
                Suffix=NULL;    //  Set Suffix machone to NULL
                multi_p=M_ALL;
        }

        void setSolutionSettings(multiply_params params) {
                multi_p=params;
                Prefix->setSolutionSettings(params);
        }

        bool operator()()//if returns true means there was a match
        {
                // Setting this boolean to true means all features including prefix, suffix, and stem are required to be extracted
                called_everything=true;
                return Prefix->operator ()();
        }

        bool operator()(item_types type)//used for detecting all word parts that start at this position; if returns true means there was a match
        {
                called_everything=false;
                this->type=type;
                //total_matches_till_now=0;
                if (type==PREFIX)
                        return Prefix->operator ()();
                else if (type==STEM)
                        return Stem->operator ()();
                else if (type==SUFFIX)
                        return Suffix->operator ()();
                else
                        return false;
        }
        bool on_match_helper();

        virtual bool on_match();

        virtual ~Enumerator()
        {
                if (Suffix)
                        delete Suffix;  // Delete Suffix machine pointer
                if (Stem)
                        delete Stem;    // Delete Stem machine pointer
                if (Prefix)
                        delete Prefix;  // Delete Prefix machine pointer
        }
};

#endif	/* _ENUMERATOR_H */

