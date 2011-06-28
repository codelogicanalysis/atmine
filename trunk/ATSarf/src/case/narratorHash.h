#ifndef NARRATORHASH_H
#define NARRATORHASH_H

#include "hadithCommon.h"
#include <QtCore/qdatastream.h>
#include <QDataStream>
#include <QMultiHash>
#include <QVector>
#include "graph_nodes.h"


inline bool possessivesCompare(const NameConnectorPrim * n1,const NameConnectorPrim * n2) {
	return n1->getString()<n2->getString();
}

class NarratorHash {
private:
	typedef QVector< NarratorPrim *> NamePrimList;
	typedef QVector<NamePrimList> NamePrimHierarchy;
	typedef QVector<NameConnectorPrim *> PossessiveList;

	class HashValue {
	public:
		HashValue(ChainNarratorNode * node, double value) {
			this->node=node;
			this->value=value;
		}
		ChainNarratorNode * node;
		double value;
	};

	typedef QMultiHash<QString,HashValue> HashTable;

	HashTable hashTable;

	QString getKey(const NamePrimHierarchy & hierarchy) {
		QString key="";
		int size=hierarchy.size();
		for (int i=0;i<size;i++) {
			int levelSize=hierarchy[i].size();
			for (int j=0; j<levelSize;j++) {
				key.append(hierarchy[i][j]->getString());
				if (j!=levelSize-1)
					key.append(" ");
			}
			if (i!=size-1)
				key.append("-");
		}
		return key;
	}
	class Visitor {
	public:
		virtual void visit(const QString & s, ChainNarratorNode * c, double value)=0;
	};
	class InsertVisitor: public Visitor {
	private:
		NarratorHash * hash;
	public:
		InsertVisitor(NarratorHash * hash) {this->hash=hash;}
		void visit(const QString & s, ChainNarratorNode * c, double value){
			hash->hashTable.insert(s,HashValue(c,value));
		}
	};
	class FindVisitor: public Visitor {
	private:
		NarratorHash * hash;
		double largestEquality;
		ChainNarratorNode * node;
	public:
		FindVisitor(NarratorHash * hash) {
			this->hash=hash;
			largestEquality=0;
			node=NULL;
		}
		void visit(const QString & s, ChainNarratorNode *, double value){
			HashTable::iterator i = hash->hashTable.find(s);
			 while (i != hash->hashTable.end() && i.key() == s) {
				 HashValue v=*i;
				 double curr_equality=v.value*value;
				 if (curr_equality>largestEquality) {
					 largestEquality=curr_equality;
					 node=v.node;
				 }
				 ++i;
			 }
		}
		double getEqualityValue() { return largestEquality; }
		ChainNarratorNode * getCorrespondingNode() { return node; }
	};

	void generateAllPosibilities(ChainNarratorNode * node, Visitor & v) {
	#ifdef NARRATORHASH_DEBUG
		qDebug()<<node->getNarrator().getString();
	#endif
		double max_equality=hadithParameters.equality_threshold*2,
			   delta=hadithParameters.equality_delta;
		Narrator * n=&node->getNarrator();
		bool abihi=isRelativeNarrator(*n);
		if (abihi)
			return; //we dont know yet to what person is the ha2 in abihi a reference so they might not be equal.
		if (n->isRasoul) {
			v.visit(alrasoul,node,max_equality);
			return;
		}
		NamePrimHierarchy names;
		PossessiveList possessives;
		int j=0; //index of names entry defined by bin
		names.append(NamePrimList());
		//qDebug()<< names.size();
		for (int i=0;i<n->m_narrator.count();i++) {
			if (n->m_narrator[i]->isNamePrim())
				names[j].append(n->m_narrator[i]);
			else {
				NameConnectorPrim * c=(NameConnectorPrim*)n->m_narrator[i];
				if (c->isPossessive()) {
					possessives.append(c);
				} else if (c->isIbn()){
					names.append(NamePrimList());
					j++;
				} else if (c->isFamilyConnector()) {
					names[j].append(c);
				}
				//if (c->isOther()) do nothing
			}
		}
		//TODO: add sorting of names inside same level (pay attention to those after family connector) and possessives
		qSort(possessives.begin(),possessives.end(),possessivesCompare);
		//TODO: allow for skipping in the selection if total number is conserved

		int levelSize=names.size(),
			possSize=possessives.size();
		for (int poss_count=0;poss_count<=possSize;poss_count++) {
			for (int level_count=1;level_count<=levelSize+1;level_count++) {
				int level_min=(level_count<=levelSize?0:1);
				int level_max=(level_count<=levelSize?level_count:names.size());
				if (level_min==1) {
					if (0==levelSize || names[0].size()==0)
						continue;
				}
				for (int names_count=1;names_count<=1/*names[level_count].size()*/;names_count++) {
					NamePrimHierarchy result;
					int actualLevelCount=0;
					if (level_min==1)
						result.append(NamePrimList());
					for (int j=level_min;j<level_max;j++) {
						NamePrimList nameList;
						for (int k=0;k<names_count && k<names[j].count();k++) {
							nameList.append(names[j][k]);
						}
						result.append(nameList);
						if (nameList.size()>0)
							actualLevelCount++;
					}
					NamePrimList possessiveList;
					for (int i=0;i<poss_count;i++) {
						possessiveList.append(possessives[i]);
					}
					if (possessiveList.size()>0)
						result.append(possessiveList);
					double value=poss_count*delta/2+actualLevelCount*max_equality/levelSize/2;
					if (result.size()>0) {
						for (int f=0;f<result.size();f++) {
							if (result[f].size()>0) { //at least one level not empty
								QString key=getKey(result);
							#ifdef NARRATORHASH_DEBUG
								qDebug()<<"\t"<<key<<"\t"<<value;
							#endif
								v.visit(key,node,value);
								break;
							}
						}
					}
				}
			}
		}
	}
public:
#if defined(REFINEMENTS) && defined(EQUALITY_REFINEMENTS) //does not work otherwise, instead of doing different versions for combinations of them on/off
	void addNode(ChainNarratorNode * node) {
		InsertVisitor v(this);
		generateAllPosibilities(node,v);
	}
	ChainNarratorNode * findCorrespondingNode(Narrator * n) {
		ChainNarratorNode * node=new ChainNarratorNode(n,-1,-1);
		FindVisitor v(this);
		generateAllPosibilities(node,v);
		//delete node;
		ChainNarratorNode * c=v.getCorrespondingNode();
	#if 0
		double val=v.getEqualityValue();
		double maxEquality=hadithParameters.equality_threshold*hadithParameters.equality_threshold;

		if (val>=maxEquality) {
			return c;
		} else {
			return NULL;
		}
	#else
		return c;
	#endif
	}
	void clear() {
		hashTable.clear();
	}

#endif
};
/*
inline double getdistance(const Narrator & n1,const Narrator & n2) //TODO: use pointers instead of expensive operations.
{

#ifdef REFINEMENTS
	bool abihi1=isRelativeNarrator(n1), abihi2=isRelativeNarrator(n2);
	if (abihi1 || abihi2)
		return max_distance; //we dont know yet to what person is the ha2 in abihi a reference so they might not be equal.
#endif
	QString n1_str=n1.getString(),n2_str=n2.getString();
	if (equal(n1_str,n2_str))
		return 0;
#ifdef REFINEMENTS
#if 1
	bool ras1=n1.isRasoul, ras2=n2.isRasoul;
#else
	bool ras1=isRasoul(n1_str), ras2=isRasoul(n2_str);
#endif
	if (ras1 && ras2)
		return 0;
	else if (ras1 || ras2)
		return max_distance;

#endif
	double dist=max_distance, delta=hadithParameters.equality_delta;
	QList<NamePrim > Names1,Names2;
	QList<NameConnectorPrim > Conns1, Conns2;
#ifdef EQUALITY_REFINEMENTS
	bool first_ibn1=false, first_ibn2=false;//means that the corresponding chain starts with ibn before any name
#endif
	for (int i=0;i<n1.m_narrator.count();i++)
		if (n1.m_narrator[i]->isNamePrim())
			Names1.append(*(NamePrim*)n1.m_narrator[i]);
		else
		{
			NameConnectorPrim & c=*(NameConnectorPrim*)n1.m_narrator[i];
			Conns1.append(c);
		#ifdef EQUALITY_REFINEMENTS
			if (Names1.size()==0)
			{
				IbnStemsDetector d(c.hadith_text,c.m_start,c.m_end);
				d();
				if (d.ibn)
					first_ibn1=true;
			}
		#endif
		}
	for (int i=0;i<n2.m_narrator.count();i++)
		if (n2.m_narrator[i]->isNamePrim())
			Names2.append(*(NamePrim*)n2.m_narrator[i]);
		else
		{
			NameConnectorPrim & c=*(NameConnectorPrim*)n2.m_narrator[i];
			Conns2.append(c);
		#ifdef EQUALITY_REFINEMENTS
			if (Names2.size()==0)
			{
				IbnStemsDetector d(c.hadith_text,c.m_start,c.m_end);
				d();
				if (d.ibn)
					first_ibn2=true;
			}
		#endif
		}

#ifdef EQUALITYDEBUG
	display("<");
	display(n1.getString());
	display(" VS ");
	display(n2.getString());
	display(">\n");
#if 0
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
#endif

	QList<EqualNamesStruct> equal_names;
	for (int i=0;i<Names1.count();i++)
	{
	#ifndef REFINEMENTS
		StemsDetector stemsD1(Names1[i].hadith_text,Names1[i].m_start,Names1[i].m_end);
		stemsD1();
	#endif
		for (int j=0;j<Names2.count();j++)
		{
		#ifndef REFINEMENTS
			StemsDetector stemsD2(Names2[j].hadith_text,Names2[j].m_start,Names2[j].m_end);//TODO: later for efficiency, perform in a seperate loop, save them then use them
			stemsD2();
			if (has_equal_stems(stemsD1.stems,stemsD2.stems))
		#else
			if (equal_ignore_diacritics(Names1[i].getString(),Names2[j].getString()))
		#endif
				equal_names.append(EqualNamesStruct(Names1[i],Names2[j],Int2(i,j)));
		}
	}
	if (equal_names.count()==0)
		return dist;
	if (equal_names.count()==min(Names1.count(),Names2.count()))
	{
		display(QString("%1,%2,%3").arg(equal_names.count()).arg(Names1.count()).arg(Names2.count()));
		display("equal names \\ ");//may be wrong will be refined later when we used EQUALITY_REFINEMENTS
		dist-=delta;
	}
#ifdef EQUALITY_REFINEMENTS
	int number_skipped_names=equal_names[0].third.difference();//start by number of skipped items
#endif
	for (int i=1;i<equal_names.count();i++)
	{
	#ifdef EQUALITY_REFINEMENTS
		int diff=equal_names[i].third.difference();
		if (diff !=number_skipped_names)
			number_skipped_names=-1;
	#endif
		if (equal_names[i-1].third.second>equal_names[i].third.second)
			return dist;
	}
	display("same order of names \\ ");
	dist-=delta; //if reaches here it means names are in correct order, otherwise it would have returned before
	QList<EqualConnsStruct> equal_conns;
#ifdef EQUALITY_REFINEMENTS
	int num_ibn1=0, num_ibn2=0;
#endif
	for (int i=0;i<Conns1.count();i++)
	{
		IbnStemsDetector stemsD1(Conns1[i].hadith_text,Conns1[i].m_start,Conns1[i].m_end);
		stemsD1();
		if (stemsD1.ibn)
		{
			Conns1.removeAt(i);
		#ifdef EQUALITY_REFINEMENTS
			num_ibn1++;
		#endif
			i--;
		}
		for (int j=0;j<Conns2.count();j++)
		{
			IbnStemsDetector stemsD2(Conns2[j].hadith_text,Conns2[j].m_start,Conns2[j].m_end);//TODO: later for efficiency, perform in a seperate loop, save them then use them
			stemsD2();
			if (stemsD2.ibn)
			{
				Conns2.removeAt(j);
			#ifdef EQUALITY_REFINEMENTS
				num_ibn2++;
			#endif
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
#ifdef EQUALITY_REFINEMENTS
	int min_names=min(Names1.count(),Names2.count()),
	min_ibn_cnt=min(num_ibn1,num_ibn2);
	bool one_ibn_first=((!first_ibn1 && first_ibn2)|| (!first_ibn2 && first_ibn1));
	bool correctly_skipped=one_ibn_first && ((first_ibn1 &&  num_ibn1==min_ibn_cnt && Names1.count()==min_names) ||
						   (first_ibn2 &&  num_ibn2==min_ibn_cnt && Names2.count()==min_names));
	if ((min_ibn_cnt+1==min_names) || (one_ibn_first && min_ibn_cnt==min_names && correctly_skipped))
	{
		if (number_skipped_names==0 && !one_ibn_first)
		{
			display("all names are there\\ ");//if all detected names are correct
			dist-=delta;//reward when all names are there
		}
		if (number_skipped_names==1 && one_ibn_first)
		{
			display("all names are there except the first and skipped by ibn \\ ");
			dist-=delta/2;//reward when starts by ibn, i.e. one name of the list is skipped
		}
	}
	else
	{//revert wrong decision previosuly if needed
		if (equal_names.count()==min(Names1.count(),Names2.count()))
		{
			display("-{equal names} \\ ");//some names are not detected and from the number of ibn, we deduce they are not equal
			dist+=delta;
		}
	}
#endif
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
	return min(max(dist-equal_conns.count()*delta/2,0.0),max_distance);
}
*/


#endif // NARRATORHASH_H
