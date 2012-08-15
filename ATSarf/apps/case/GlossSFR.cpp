
#include "GlossSFR.h"


GlossSFR::GlossSFR(QString *text) : Stemmer(text,0)
{
    this->text = *text;
     _LIST<<"flower"<<"nose"<<"smell"<<"rose";
};


bool GlossSFR::on_match()
{
    bool RELATED=false;

    for(int i =0; i<_LIST.size(); i++)
    {
        QString _desc = stem_info->description();
        if(stem_info->description().contains(_LIST[i],Qt::CaseInsensitive))
        {
            RELATED = true;
            break;
        }
    }
    bool Present = RelatedW.contains(this->text,Qt::CaseInsensitive);
    if(RELATED && !Present)
    {
        RelatedW += this->text;
        theSarf->out<<this->text<<'\n';
    }
    return true;
};

