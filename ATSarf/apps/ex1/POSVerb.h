#ifndef POSVERB_H
#define POSVERB_H

#include<stemmer.h>
#include<QTextStream>

/**
 * @class POSVerb
 * @brief The following class defines the parameters and functions required to extract the words with possible verb part of speech tags
 * @author Ameen Jaber
 */
class POSVerb : public Stemmer
{
private:
    /// String that holds the input text
    QString text;
    /// String that holds the previous solutions which were accepted as verbs in order to prevent repitition
    QString RelatedW;
    /// TextStream that stores the different solutions extracted by the functions
    //QTextStream & result;

public:

    /**
     * This is the constructor of the POSVerb class
     * @param text This is the input text to be processed
     */
    explicit POSVerb(QString * text/*, QTextStream & r*/);

    /**
     * This is the function called by the tool upon finding a possible solution for the input. By implementing this method, we control the accepted solutions that meet
     * our functionality required.
     * @return This function returns a boolean that is processed by the tool, as long as the returned value is true the tool continues to find the next possible
     * solution till all possible ones and extracted, else if the returned value is false it will stop the analysis process.
     */
    bool on_match();
};

#endif // POSVERB_H
