#include "narrator_abstraction.h"
#include <QStringList>
QTextStream chainOut;
#define PRINTTEXT

NarratorPrim::NarratorPrim(){
    m_start=0;
    m_end=0;
    }

NarratorPrim::NarratorPrim(long long m_start){
this->m_start=m_start;
}
//
//Narrator::Narrator(){
//m_narrator=new  QList <NarratorPrim *> ();
//}
void Narrator::serialize(QTextStream &chainOut,QStringList &wordList) const{

    chainOut<<"\t\t\t\t\t\tNarrator {\n";
    int size=m_narrator.size();
    for (int i=0;i<size;i++)
    {
        chainOut<<"\t\t\t\t\t\t";
        m_narrator[i]->serialize(chainOut,wordList);

        chainOut<<m_narrator[i]->m_start<<" "<<m_narrator[i]->m_end<<"";

#ifdef PRINTTEXT
        chainOut<<"\t\t";
        for (int  j=m_narrator[i]->m_start;j <= m_narrator[i]->m_end;j++){
            chainOut<<wordList[j]<<" ";
        }
        chainOut<<"\n";
#endif
    }
    chainOut<<"\t\t\t\t\t\t\t}\n";
}

NamePrim::NamePrim():NarratorPrim(){}
NamePrim::NamePrim(long long m_start):NarratorPrim(m_start){}
void NamePrim::serialize(QTextStream &chainOut,QStringList &wordList) const{

    chainOut<<"\tName ";
}

NameConnectorPrim::NameConnectorPrim():NarratorPrim(){}
NameConnectorPrim::NameConnectorPrim(long long m_start):NarratorPrim(m_start){}

NarratorConnectorPrim::NarratorConnectorPrim(){
m_start=0;
m_end=0;
}
void NameConnectorPrim::serialize(QTextStream &chainOut,QStringList &wordList) const{

    chainOut<<"\t\tName Conn. ";
}

NarratorConnectorPrim::NarratorConnectorPrim(long long m_start){
this->m_start=m_start;
}

void NarratorConnectorPrim::serialize(QTextStream &chainOut,QStringList &wordList) const{

    chainOut<<"\t\t\t\t\t\tNarrator Connector: "<<m_start<<" "<<m_end<<"";


#ifdef PRINTTEXT
            chainOut<<"\t\t";
            for (int i=this->m_start;i<=this->m_end;i++){
                chainOut<<wordList[i]<<" ";
            }
           chainOut<<"\n";
#endif
}



void Chain::serialize(QTextStream &chainOut,QStringList &wordList) const{


        int size=m_chain.size();
        chainOut<<"\t\t\t\t\tCH {\n";
        for (int i=0;i<size;i++)
        {
            m_chain[i]->serialize(chainOut,wordList);
        }
        chainOut<<"\t\t\t\t\t}\n\n";

}
