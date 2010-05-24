#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "legend.h"


Legend::Legend()
{ID = -1;
    textColor = QColor(0,0,0);
    bckgColor = QColor(255,255,255);
    italic = 0;
    bold = 0;
    underlined = 0;
   //  Legend(-1,QColor(0,0,0),QColor(255,255,255));
}

Legend::Legend(int i)
{
    Legend(i,QColor(0,0,0),QColor(255,255,255));
}

Legend::Legend(int i, QColor tc, QColor bc)
{
    Legend(i,tc,bc,false,false,false);
}

Legend::Legend(int i, QColor tc, QColor bc, bool it, bool bl, bool un)
{
    ID = i;
    textColor = tc;
    bckgColor = bc;
    italic = it;
    bold = bl;
    underlined = un;
}

void Legend::setID(int i)
{
    ID = i;
}

void Legend::setTextColor(QColor tc)
{
    textColor = tc;
}

void Legend::setBckgColor(QColor bc)
{
    bckgColor = bc;
}

void Legend::setItalic(bool it)
{
    italic = it;
}

void Legend::setBold(bool bl)
{
    bold = bl;
}

void Legend::setUnderlined(bool un)
{
    underlined = un;
}

int Legend::getID()
{
    return ID;
}

QColor Legend::getTextColor()
{
    return textColor;
}

QColor Legend::getBckgColor()
{
    return bckgColor;
}

bool Legend::isItalic()
{
    return italic;
}

bool Legend::isBold()
{
    return bold;
}

bool Legend::isUnderlined()
{
    return underlined;
}
void Legend::setSource(QString s)
{source=s;}

QString Legend::getSource()
{return source;
}


bool Legend::operator==(const Legend &l1)
{
    if ((l1.ID == ID)&&(l1.textColor == textColor)&&(l1.bckgColor == bckgColor)&&(l1.italic == italic)&&(l1.bold == bold) &&(l1.underlined == underlined))
        return 1;

        return 0;

}
