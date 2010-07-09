#include "narrator_abstraction.h"
#include <QStringList>
#include <assert.h>

qint8 getType(const NarratorPrim *)
{
	return 'n';
}
qint8 getType(const NamePrim *)
{
	return 'N';
}
qint8 getType(const ChainPrim *)
{
	return 'C';
}
qint8 getType(const NameConnectorPrim *)
{
	return 'c';
}
qint8 getType(const Narrator *)
{
	return 'a';
}
qint8 getType(const NarratorConnectorPrim *)
{
	return 'p';
}
qint8 getType(const Chain *)
{
	return 'A';
}

NarratorPrim::NarratorPrim(QString * hadith_text){
    m_start=0;
    m_end=0;
	this->hadith_text=hadith_text;
}

NarratorPrim::NarratorPrim(QString * hadith_text,long long m_start){
	this->m_start=m_start;
	this->hadith_text=hadith_text;
}

//
//Narrator::Narrator(){
//m_narrator=new  QList <NarratorPrim *> ();
//}
Narrator::Narrator(QString * hadith_text):ChainPrim(hadith_text){}
void Narrator::serialize(QTextStream &chainOut) const{
	chainOut<<"\tNarrator {\n";
    int size=m_narrator.size();
    for (int i=0;i<size;i++)
	{
		chainOut<<"\t";
		m_narrator[i]->serialize(chainOut);

		//chainOut<<m_narrator[i]->m_start<<" "<<m_narrator[i]->m_end<<"";
		chainOut<<"\t";
		chainOut<<hadith_text->mid(m_narrator[i]->m_start, m_narrator[i]->m_end-m_narrator[i]->m_start+1)<<" ";
		chainOut<<"\n";
	}
	chainOut<<"\t\t}\n";
}
void Narrator::serialize(QDataStream &chainOut) const{
	chainOut<<getType((const Narrator*)this);
	int size=m_narrator.size();
	chainOut<<(qint32)size;
	for (int i=0;i<size;i++)
	{
		m_narrator[i]->serialize(chainOut);
		chainOut<<m_narrator[i]->m_start<<m_narrator[i]->m_end;
	}
}

void Narrator::deserialize(QDataStream &chainIn) {
	/*qint8 c;
	chainIn>>c;
	assert(c==getType((const Narrator*)this));*/
	qint32 size;
	chainIn>>size;
	for (int i=0;i<size;i++)
	{
		NarratorPrim * narrator_prim;
		qint8 c;
		chainIn>>c;
		if (c==getType((const NameConnectorPrim*)narrator_prim))
			narrator_prim=new NameConnectorPrim(hadith_text);
		else if (c==getType((const NamePrim*)narrator_prim))
			narrator_prim=new NamePrim(hadith_text);
		else
			throw "stop";
		narrator_prim->deserialize(chainIn);
		chainIn>>narrator_prim->m_start>>narrator_prim->m_end;
		m_narrator.append(narrator_prim);
	}
}

NamePrim::NamePrim(QString * hadith_text):NarratorPrim(hadith_text){}
NamePrim::NamePrim(QString * hadith_text,long long m_start):NarratorPrim(hadith_text,m_start){}

void NamePrim::serialize(QTextStream &chainOut) const{
    chainOut<<"\tName ";
}
void NamePrim::serialize(QDataStream & chainOut) const{
	chainOut<<getType((const NamePrim*)this);
}
void NamePrim::deserialize(QDataStream &) {
	/*qint8 c;
	chainIn>>c;
	assert(c==getType((const NamePrim*)this));*/
}

NameConnectorPrim::NameConnectorPrim(QString * hadith_text):NarratorPrim(hadith_text){}
NameConnectorPrim::NameConnectorPrim(QString * hadith_text,long long m_start):NarratorPrim(hadith_text,m_start){}

void NameConnectorPrim::serialize(QTextStream &chainOut) const
{
	chainOut<<"\t\tName Conn. ";
}
void NameConnectorPrim::serialize(QDataStream &chainOut) const{
	chainOut<<getType((const NameConnectorPrim*)this);
}
void NameConnectorPrim::deserialize(QDataStream &) {
	/*qint8 c;
	chainIn>>c;
	assert(c==getType((const NameConnectorPrim*)this));*/
}

NarratorConnectorPrim::NarratorConnectorPrim(QString * hadith_text):ChainPrim(hadith_text){
	m_start=0;
	m_end=0;
	this->hadith_text=hadith_text;
}

NarratorConnectorPrim::NarratorConnectorPrim(QString * hadith_text,long long m_start):ChainPrim(hadith_text){
	this->m_start=m_start;
}

void NarratorConnectorPrim::serialize(QTextStream &chainOut) const{
	chainOut<<"\tNarrator Connector: ";//<<m_start<<" "<<m_end<<"";
	chainOut<<"\t\t";
	chainOut<<hadith_text->mid(this->m_start,this->m_end-this->m_start+1)<<" ";
	chainOut<<"\n";
}
void NarratorConnectorPrim::serialize(QDataStream &chainOut) const{
	chainOut<<getType((const NarratorConnectorPrim*)this);
	chainOut<<(qlonglong)m_start<<(qlonglong)m_end;
}
void NarratorConnectorPrim::deserialize(QDataStream &chainIn){
	/*qint8 c;
	chainIn>>c;
	assert(c==getType((const NarratorConnectorPrim*)this));*/
	qlonglong start,end;
	chainIn>>start>>end;
	m_start=start;
	m_end=end;
}
Chain::Chain(QString * hadith_text)
{
	this->hadith_text=hadith_text;
}
void Chain::serialize(QTextStream & chainOut) const
{
	int size=m_chain.size();
	chainOut<<" CH {\n";
	for (int i=0;i<size;i++)
		m_chain[i]->serialize(chainOut);
	chainOut<<"}\n\n";
}
void Chain::serialize(QDataStream &stream) const
{
	stream<<getType((const Chain*)this);
	int size=m_chain.size();
	stream<<(qint32)size;
	for (int i=0;i<size;i++)
		m_chain[i]->serialize(stream);
}
void Chain::deserialize(QDataStream &stream)
{
	qint8 c;
	stream>>c;
	assert(c==getType((const Chain*)this));
	qint32 size;
	stream>>size;
	for (int i=0;i<size;i++)
	{
		ChainPrim * chain_prim;
		qint8 c;
		stream>>c;
		if (c==getType((const NarratorConnectorPrim*)chain_prim))
			chain_prim=new NarratorConnectorPrim(hadith_text);
		else if (c==getType((const Narrator*)chain_prim))
			chain_prim=new Narrator(hadith_text);
		else
			throw "stop";
		chain_prim->deserialize(stream);
		m_chain.append(chain_prim);
	}
}
ChainPrim::ChainPrim(QString* hadith_text)
{
	this->hadith_text=hadith_text;
}
/*
void ChainPrim::serialize(QDataStream & chainOut) const
{
	chainOut<<getType((const ChainPrim*)this);
}
void ChainPrim::deserialize(QDataStream & chainIn)
{
	qint8 c;
	chainIn>>c;
	assert(c==getType((const ChainPrim*)this));
}
void ChainPrim::serialize(QTextStream &) const {}*/


QDataStream &operator>>(QDataStream &in, ChainNarratorPrim &p)
{
	p.deserialize(in);
	return in;
}

QDataStream &operator<<(QDataStream &out, const ChainNarratorPrim &p)
{
	p.serialize(out);
	return out;
}
