#include "hadith.h"
#include "narrator_abstraction.h"
#include <QStringList>
#include <assert.h>
#include "Triplet.h"
#include "stemmer.h"
#include <QPair>

#ifdef EQUALITYDEBUG
inline void display(QString t)
{
	out<<t;
	//qDebug() <<t;
}
#else
#define display(c) ;
#endif

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
bool Narrator::isNarrator() const
{
	return true;
}
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

bool NamePrim::isNamePrim() const {
	return true;
}
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

bool NameConnectorPrim::isNamePrim() const {
	return false;
}
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

bool NarratorConnectorPrim::isNarrator() const
{
	return false;
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

typedef QPair<long long, long> ItemIDCategory;
typedef QVector<ItemIDCategory> StemList;
class StemsDetector: public Stemmer
{
private:
	int forced_finish_pos;
public:
	StemList stems;

	StemsDetector(QString * text, int start,int finish):Stemmer(text,start,false)
	{
		stems.clear();
		forced_finish_pos=finish;
		multi_p.abstract_category=false;
		multi_p.POS=false;
		multi_p.description=false;
		multi_p.raw_data=false;
		setSolutionSettings(multi_p);
	}
	bool on_match()
	{
		if (info.finish==forced_finish_pos)//TODO: check if is Name or save initially all stems without re-stemming
		{
			ItemIDCategory result(Stem->id_of_currentmatch,Stem->category_of_currentmatch);
			stems.append(result);
		}
		return true;
	}
};

inline bool has_equal_stems(StemList l1,StemList l2)
{
	for (int i=0;i<l1.count();i++)
	{
		for (int j=0;j<l2.count();j++)
		{
			//qDebug()<<l1[i]<<"-"<<l2[j];
			if (l1[i]==l2[j])
				return true;
		}
	}
	return false;
}

inline bool has_common_element(QList<QString> l1,QList<QString> l2)
{
	for (int i=0;i<l1.count();i++)
	{
		for (int j=0;j<l2.count();j++)
		{
			//qDebug()<<l1[i]<<"-"<<l2[j];
			if (l1[i]==l2[j])
				return true;
		}
	}
	return false;
}

class IbnStemsDetector: public Stemmer
{
private:
	int forced_finish_pos;
public:
	bool ibn, place, city,country;
	QList<QString> country_names,city_names;
	StemList stems;

	IbnStemsDetector(QString * text, int start,int finish):Stemmer(text,start,false)
	{
		stems.clear();
		forced_finish_pos=finish;
		multi_p.abstract_category=true;
		multi_p.POS=true;
		multi_p.description=true;
		multi_p.raw_data=true;
		ibn=false;
		city=false;
		place=false;
		country=false;
		setSolutionSettings(multi_p);
	}
	bool on_match()
	{
		if (info.finish==forced_finish_pos)
		{
			ItemIDCategory result(Stem->id_of_currentmatch,Stem->category_of_currentmatch);
			stems.append(result);
			if (!ibn)
			{
				solution_position * stem_sol=Stem->computeFirstSolution();
				do
				{
#ifdef PREPROCESS_DESCRIPTIONS
					if (NMC_descriptions.contains(Stem->solution->description_id))
#else
					if (Stem->solution->description()=="son")
#endif
					{
						ibn=true;
						break;
					}
					else if (Stem->solution->abstract_categories.getBit(bit_PLACE))
					{
						place=true;
						//display (Stem->solution->raw_data +":"+Stem->solution->description().append("\n"));
						if (Stem->solution->abstract_categories.getBit(bit_CITY))
						{
							city=true;
							//display("{city}\n");
							city_names.append(Stem->solution->raw_data);
							country_names.append(Stem->solution->description().split("city/town in ").at(1));
						}
						if (Stem->solution->abstract_categories.getBit(bit_COUNTRY))
						{
							//display("{country}\n");
							country=true;
							QString desc=Stem->solution->description();
							if (Stem->solution->abstract_categories.getBit(bit_POSSESSIVE))
								country_names.append(desc.split("possessive form of ").at(1));
							else
								country_names.append(desc);
						}
						break;
					}
				}while(Stem->computeNextSolution(stem_sol));
				delete stem_sol;
			}
		}
		return true;
	}
};

typedef Triplet<NamePrim,NamePrim, int> EqualNamesStruct;
typedef Triplet<NameConnectorPrim,NameConnectorPrim, int> EqualConnsStruct;
double getdistance(const Narrator & n1,const Narrator & n2)
{
	double dist=1, delta=0.4;
	QList<NamePrim> Names1,Names2;
	QList<NameConnectorPrim> Conns1, Conns2;
	for (int i=0;i<n1.m_narrator.count();i++)
		if (n1.m_narrator[i]->isNamePrim())
			Names1.append(*(NamePrim*)n1.m_narrator[i]);
		else
			Conns1.append(*(NameConnectorPrim*)n1.m_narrator[i]);
	for (int i=0;i<n2.m_narrator.count();i++)
		if (n2.m_narrator[i]->isNamePrim())
			Names2.append(*(NamePrim*)n2.m_narrator[i]);
		else
			Conns2.append(*(NameConnectorPrim*)n2.m_narrator[i]);

#ifdef EQUALITYDEBUG
	for (int i=0;i<Names1.count();i++)
		display(Names1[i].getString()+" - ");
	display("\n");
	for (int i=0;i<Conns1.count();i++)
		display(Conns1[i].getString()+" - ");
	display("\n");
	for (int i=0;i<Names2.count();i++)
		display(Names2[i].getString()+" - ");
	display("\n");
	for (int i=0;i<Conns2.count();i++)
		display(Conns2[i].getString()+" - ");
	display("\n");
#endif

	QList<EqualNamesStruct> equal_names;
	for (int i=0;i<Names1.count();i++)
	{
		StemsDetector stemsD1(Names1[i].hadith_text,Names1[i].m_start,Names1[i].m_end);
		stemsD1();
		for (int j=0;j<Names2.count();j++)
		{
			StemsDetector stemsD2(Names2[j].hadith_text,Names2[j].m_start,Names2[j].m_end);//TODO: later for efficiency, perform in a seperate loop, save them then use them
			stemsD2();
			if (has_equal_stems(stemsD1.stems,stemsD2.stems))
				equal_names.append(EqualNamesStruct(Names1[i],Names2[j],j));
		}
	}
	if (equal_names.count()==0)
		return dist;
	if (equal_names.count()==min(Names1.count(),Names2.count()))
	{
		display("equal names \\ ");
		dist-=delta;
	}
	for (int i=1;i<equal_names.count();i++)
		if (equal_names[i-1].third>equal_names[i].third)
			return dist;
	display("same order of names \\ ");
	dist-=delta; //if reaches here it means names are in correct order, otherwise it would have returned before
	QList<EqualConnsStruct> equal_conns;
	for (int i=0;i<Conns1.count();i++)
	{
		IbnStemsDetector stemsD1(Conns1[i].hadith_text,Conns1[i].m_start,Conns1[i].m_end);
		stemsD1();
		if (stemsD1.ibn)
		{
			Conns1.removeAt(i);
			i--;
		}
		for (int j=0;j<Conns2.count();j++)
		{
			IbnStemsDetector stemsD2(Conns2[j].hadith_text,Conns2[j].m_start,Conns2[j].m_end);//TODO: later for efficiency, perform in a seperate loop, save them then use them
			stemsD2();
			if (stemsD2.ibn)
			{
				Conns2.removeAt(j);
				j--;
			}
			//TODO: here check if mutually exclusive and if so, punish
			if (stemsD1.city && stemsD2.city )
			{
				if (!has_common_element(stemsD1.country_names,stemsD2.country_names))
				{
					display("cities located in different countries \\ ");
					dist+=delta;
				}
				else
				{
					if (!has_common_element(stemsD1.city_names,stemsD2.city_names))
					{
						display("different cities located in same country \\ ");
						dist+=delta;
					}
					else
					{
						display("same city \\ ");
						dist-=delta;
					}
				}
				continue;
			}
			else if ((stemsD1.city && stemsD2.country) || (stemsD2.city && stemsD1.country))
			{
				if (!has_common_element(stemsD1.country_names,stemsD2.country_names))
				{
					display(QString("city and country mutually exclusive \\")); // %1 VS %2\\ ").arg(stemsD1.country_names[0]).arg(stemsD2.country_names[0]));
					dist+=delta;
				}
				else
				{
					display("city contained in country \\ ");
					dist-=delta/2;
				}
				continue;
			}
			else if (stemsD2.country && stemsD1.country)
			{
				if (!has_common_element(stemsD1.country_names,stemsD2.country_names))
				{
					display("different countries \\ ");
					dist+=delta;
				}
				else
				{
					display("same countries \\ ");
					dist-=delta;
				}
				continue;
			}

			//TODO: maybe punish more and also add more different rules other than places
			if (has_equal_stems(stemsD1.stems,stemsD2.stems) && !stemsD1.ibn && !stemsD2.ibn) //ibn checks redundant removed before
				equal_conns.append(EqualConnsStruct(Conns1[i],Conns2[j],j));
			//TODO: ibn must have a seperate rule which is inter-related to names
		}
	}
	if (Conns1.count()==0 || Conns2.count()==0)
	{
		display("No connectors \\ ");
		dist-=delta;
	}
	if (equal_conns.count()>0)
	{
		display(QString("%1 identical connectors :{ ").arg(equal_conns.count()));
	#ifdef EQUALITYDEBUG
		for (int i=0;i<equal_conns.count();i++)
			display(equal_conns[i].first.getString()+" - ");
		display("} \\");
	#endif
		dist-=delta*equal_conns.count(); //reward as much as there are identical connectors other than ibn
	}
	display("\n");
	return min(max(dist-equal_conns.count()*delta/2,0.0),1.0);
}
double equal(const Narrator & n1,const  Narrator  & n2)
{
	return 1 - getdistance(n1,n2);
}

double Narrator::equals(const Narrator & rhs) const
{
    return equal(*this,rhs);
}
