#include "POSVerb.h"

POSVerb::POSVerb(QString *text, QTextStream &r) : Stemmer(text,0), result(r)
{
    this->text = *text;
};


bool POSVerb::on_match()
{
    if(stem_info->POS.contains("VERB", Qt::CaseInsensitive) )
    {
        if (!(RelatedW.contains(this->text,Qt::CaseInsensitive)))
        {
            RelatedW += this->text;
            result<<this->text<<'\n';
            //out<<this->text<<'\n';
        }
    }
    return true;
};
