#include "narrator_abstraction.h"
#include "logger/logger.h"


NarratorPrim::NarratorPrim(){
    m_start=0;
    m_end=0;
    }
NarratorPrim::NarratorPrim(long long m_start){
this->m_start=m_start;
}

NamePrim::NamePrim():NarratorPrim(){}
NamePrim::NamePrim(long long m_start):NarratorPrim(m_start){}

NameConnectorPrim::NameConnectorPrim():NarratorPrim(){}
NameConnectorPrim::NameConnectorPrim(long long m_start):NarratorPrim(m_start){}

NarratorConnectorPrim::NarratorConnectorPrim(){
m_start=0;
m_end=0;
}

NarratorConnectorPrim::NarratorConnectorPrim(long long m_start){
this->m_start=m_start;
}

void Chain::serialize(){
        QFile chainOutput("test-cases/chainOutput");
        if (!chainOutput.open(QIODevice::ReadWrite))
                return ;

        QTextStream chainOut(&chainOutput);
        chainOut.setCodec("utf-8");

        int size=m_chain.size();
        for (int i=0;i<size;i++)
        {
            chainOut<<"Chain "<<i<<"{\n";

        }
}
