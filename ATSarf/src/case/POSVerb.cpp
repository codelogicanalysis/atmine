#include "POSVerb.h"

POSVerb::POSVerb(QString *text) : Stemmer(text,0)
{
    this->text = *text;
};


bool POSVerb::on_match()
{
    if(stem_info->POS.contains("ADJ", Qt::CaseInsensitive) )
    {
        if (!(RelatedW.contains(this->text,Qt::CaseInsensitive)))
        {
            RelatedW += this->text;
            out<<this->text<<'\n';
        }
    }
    return true;
};
