/**
  * @file   stemmer.h
  * @brief  this header file contains the definition of the stemmer class which extracts the morphological features of a words
  * @author Jad Makhlouta
  */

/**
 * @mainpage    ATSarf Documentation
 * @author      Ameen Jaber
 * @brief      The aim of this project is to provide an arabic morphological analyzer. Given a word, this tool returns the
 *  different morphological features of it such as stem, prefix, suffix, POS, tec.
 * @date 20-5-2012
 */

#ifndef STEMMER_H
#define STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "common.h"
#include <QVector>

/**
  * @class  SarfParameters
  * @brief  This class defines the parameters of operation of the Sarf program in order to enable/disable multi-word entry at the input
  * @author Jad Makhlouta
  */
class SarfParameters {
    public:
        /// Boolean in order to enable/disable run on words
        bool enableRunonwords;

        /**
          * This is the constructor of the SarfParameters class which by default sets the run on words capability to false
          */
        SarfParameters() {
            enableRunonwords = false;
        }
};
extern SarfParameters sarfParameters;

class Stemmer;

/**
  * @class  PrefixMachine
  * @author Jad Makhlouta
  * @brief  This class (PrefixMachine) inherets from the PrefixSearch class, and it implements the Prefix extraction routine
  */
class PrefixMachine: public PrefixSearch { //TreeMachine
    public:
        Stemmer *controller;

        /**
          * This method implements the constructor of the PrefixMachine which in turn calls the PrefixSearch constructor; the class which PrefixMachine inherets from.
          * @param  controller
          * @param  start   index of the input string to be analyzed
          */
        PrefixMachine(Stemmer *controller, int start);
        /**
          * This method is called upon finding a Prefix Match
          * @return It returns true if a prefix match is found, else false
          */
        bool onMatch();
        /**
          * This method implements the destructor of the PrefixMachine
          */
        virtual ~PrefixMachine() {}
};

/**
  * @class  StemMachine
  * @author Jad Makhlouta
  * @brief  This class (StemMachine) inherets from the StemSearch class, and it implements the Stem extraction routine
  */
class StemMachine: public StemSearch {
    public:
        Stemmer *controller;

        /**
          * This method implements the constructor of the StemMachine which in turn calls the StemSearch constructor; the class which StemMachine inherets from.
          * @param  controller
          * @param  start   index of the input at which the StemMachine is required to operate
          * @param  prefix_category
          */
        StemMachine(Stemmer *controller, int start, long prefix_category);
        //bool shouldcall_onmatch(int position);

        /**
          * This method is called upon finding a Stem Match
          * @return It returns true if a prefix match is found, else false
          */
        bool onMatch();
        /**
          * This method implements the destructor of the StemMachine
          */
        virtual ~StemMachine() {}
};

/**
  * @class  SuffixMachine
  * @author Jad Makhlouta
  * @brief  This class (SuffixMachine) inherets from the SuffixSearch class, and it implements the Suffix extraction routine
  */
class SuffixMachine: public SuffixSearch {
    public:
        Stemmer *controller;

        /**
          * This method implements the constructor of the SuffixMachine
          * @param  controller
          * @param  start   index of the input at which the SuffixMachine is required to operate
          * @param  prefix_category
          * @param  stem_category
          */
        SuffixMachine(Stemmer *controller, int start, long prefix_category, long stem_category);


        bool shouldcall_onmatch(int position);

        /**
          * This method is called upon finding a Suffix Match
          * @return It returns true if a prefix match is found, else false
          */
        bool onMatch();
        /**
          * This method implements the destructor of the SuffixMachine
          */
        virtual ~SuffixMachine() {}
};

/**
  * @class  SubMachines
  * @author Jad Makhlouta
  * @brief  This class (SubMachines) is declared and used in case the multi-input mode is used where multiple
  * consecutive words can be entered and the morphological analyzer processes them a functional mode which is
  * normally not used
  */
class SubMachines {
    public:
        /// Prefix extractor
        PrefixMachine *Prefix;

        /// Stem extractor
        StemMachine *Stem;

        /// Suffix extractor
        SuffixMachine *Suffix;

        /**
         *  This function is the contructor of the SubMachines class
         *  @param  Prefix  pointer to the PrefixMachine
         *  @param  Stem    pointer to the StemMachine
         *  @param  Suffix  pointer to the SuffixMachine
         */
        SubMachines(PrefixMachine *Prefix, StemMachine *Stem, SuffixMachine *Suffix) {
            this->Prefix = Prefix;
            this->Stem = Stem;
            this->Suffix = Suffix;
        }

};

/**
  * @class  Stemmer
  * @author Jad Makhlouta
  * @brief This class implements the methods that extract the different morphological features of an arabic string input such as the stem, prefix, and suffix
  */
class Stemmer {
    public://protected:

        /// This structure holds the settings required which specify the details of the solution
        multiply_params multi_p;

        /// Type of data required to be extracted by the user such as Prefix,Stem, or Suffix
        item_types type;

        /// Indicates whether a detailed solution of the input is required or not
        bool get_all_details;
        //for use in on_match()

        /// Vector containing the final result of prefixes of a word and their details based on a single interpretation of the input
        QVector<minimal_item_info> *prefix_infos;

        /// Contains the stem info for a corresponding interpretation of the input word
        minimal_item_info *stem_info;

        /// Vector containing the final result of suffixes of a word and their details based on a single interpretation of the input
        QVector<minimal_item_info> *suffix_infos;

        QStringRef getString() {
            return info.text->midRef(info.start, info.finish - info.start + 1);
        }
    private:
        int runwordIndex;
        QList<SubMachines> machines;
        friend class SuffixMachine;
    private:
        void removeLastMachines();
    public:
        /// Boolean indicating whether all machines are called or not based on user input
        bool called_everything;

        /// Define a pointer to PrefixMachine which extracts the prefix of the word
        PrefixMachine *Prefix;

        /// Define a pointer to StemMachine which extracts the stem of the word
        StemMachine *Stem;

        /// Define a pointer to SuffixMachine which extracts the suffic of the word
        SuffixMachine *Suffix;

        /// Structure in which the input string is stored with the start/finish index
        text_info info;

        /**
          * This is the constructor of the Stemmer class
          * @param  text    pointer to the input text
          * @param  start   integer refering to the starting index
          * @param  get_all_details boolean that specifies whether all details of the solution are required or not, it is set by default to true
          */
        Stemmer(QString *text, int start, bool get_all_details = true) {
            // Initialize the prefix,stem, and suffix info to null
            prefix_infos = NULL;
            stem_info = NULL;
            suffix_infos = NULL;
            info.start = start; // setting start index value taken from parameters
            info.finish = start; // setting finish index
            info.text = text; // initialize text in text to input
            this->get_all_details = get_all_details; // checking from input whether to get all details of word morphology
            Prefix = new PrefixMachine(this,
                                       start); // Initializing the Prefix machine passing start index and pointer to current class
            Stem = NULL; // Set Stem machine to NULL
            Suffix = NULL;  //  Set Suffix machone to NULL
            multi_p = M_ALL;
            runwordIndex = 0;

            if (!sarfParameters.enableRunonwords) {
                machines.append(SubMachines(NULL, NULL, NULL));    //just to fill it with anything so that in on_match machine.size()==1
            }
        }

        /**
          * This method sets the settings of the solution required in terms of the parameters returned
          * @param  params This is of type multiply_params which holds the settings required
          */
        void setSolutionSettings(multiply_params params) {
            multi_p = params;
            Prefix->setSolutionSettings(params);
        }

        /**
          * This method defines the bracket operator overload to initialize the Prefix machine
          * @return boolean with true value if match found else false
          */
        bool operator()() { //if returns true means there was a match
            // Setting this boolean to true means all features including prefix, suffix, and stem are required to be extracted
            called_everything = true;
            return Prefix->operator()();
        }

        /**
          * This method defines bracket operator for a specific type of required data to be extracted
          * such as Prefix, Suffix, or Stem
          * @param  type Variable of type item_types which is a typedef
          */
        bool operator()(item_types
                        type) { //used for detecting all word parts that start at this position; if returns true means there was a match
            called_everything = false;
            this->type = type;

            //total_matches_till_now=0;
            if (type == PREFIX) {
                return Prefix->operator()();
            } else if (type == STEM) {
                return Stem->operator()();
            } else if (type == SUFFIX) {
                return Suffix->operator()();
            } else {
                return false;
            }
        }
        bool on_match_helper();

        /**
          * This method is called upon finding a match for the input word
          */
        virtual bool on_match();

        /**
          * This method defines the destructor of the Stemmer class
          */
        virtual ~Stemmer() {
            if (Suffix) {
                delete Suffix;    // Delete Suffix machine pointer
            }

            if (Stem) {
                delete Stem;    // Delete Stem machine pointer
            }

            if (Prefix) {
                delete Prefix;    // Delete Prefix machine pointer
            }
        }
};
inline void printMachines(QList<SubMachines>) {}

#endif

