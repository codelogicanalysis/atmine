#include "sarftag.h"
#include "global.h"
#include "amtmainwindow.h"

class AMTMainWindow;

SarfTag::SarfTag(int start, int length, QString *text, QWidget *parent): Stemmer(text,0)
{
    this->text = *text;
    this->start = start;
    this->length = length;
}

bool SarfTag::on_match() {

    for( int i=0; i<_atagger->tagTypeVector->count(); i++) {
        if(stem_info->POS.contains((_atagger->tagTypeVector->at(i)).tag, Qt::CaseInsensitive) )
        {
            if (!(RelatedW.contains(this->text,Qt::CaseInsensitive)))
            {
                QString tag = (_atagger->tagTypeVector->at(i)).tag;
                RelatedW += this->text;
                /*
                dflkmo
                */
                _atagger->insertSarfTag(tag,start,length,sarf);
                //((AMTMainWindow*)parent)->tagWord(start,length,fgcolor,bgcolor,font,underline,italic,bold);
            }
            break;
        }
    }
    return true;
}
