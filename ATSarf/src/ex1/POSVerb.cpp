#include "POSVerb.h"

/**
 * This is the constructor of the POSVerb class
 * @param text This is the input text to be processed
 * @param r	
 */
POSVerb::POSVerb(QString *text, QTextStream &r) : Stemmer(text,0), result(r)
{
    this->text = *text;
};

/**
  * This is the function called by the tool upon finding a possible solution for the input. By implementing this method, we control the accepted solutions that meet
  * meets our functionality required.
  * @return This function returns a boolean that is processed by the tool, as long as the returned value is true the tool continues to find the next possible
  * solution till all possible ones and extracted, else if the returned value is false it will stop the analysis process.
  */
bool POSVerb::on_match()
{
    if(stem_info->POS.contains("VERB", Qt::CaseInsensitive) )
    {
        if (!(RelatedW.contains(this->text,Qt::CaseInsensitive)))
        {
            RelatedW += this->text;
            result<<this->text<<endl;
        }
    }
    return true;
};
