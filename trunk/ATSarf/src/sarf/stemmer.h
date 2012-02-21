#ifndef _STEMMER_H
#define	_STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "common.h"
#include <QVector>

class SarfParameters {
public:
	bool enableRunonwords;
	SarfParameters() {
		enableRunonwords=false;
	}
};
extern SarfParameters sarfParameters;

class Stemmer;

class PrefixMachine: public PrefixSearch//TreeMachine
{
public:
	Stemmer * controller;

	PrefixMachine(Stemmer * controller,int start);
	bool onMatch();
	virtual ~PrefixMachine(){}
};

class StemMachine: public StemSearch
{
public:
	Stemmer * controller;

	StemMachine(Stemmer * controller,int start,long prefix_category);
	//bool shouldcall_onmatch(int position);
	bool onMatch();
	virtual ~StemMachine(){}
};

class SuffixMachine: public SuffixSearch
{
public:
	Stemmer * controller;

	SuffixMachine(Stemmer * controller,int start, long prefix_category,long stem_category);
	bool shouldcall_onmatch(int position);
	bool onMatch();
	virtual ~SuffixMachine(){}
};

#ifdef RUNON_WORDS
class SubMachines
{
public:
	PrefixMachine* Prefix;
	StemMachine* Stem;
	SuffixMachine* Suffix;

	SubMachines(PrefixMachine* Prefix, StemMachine* Stem, SuffixMachine* Suffix)
	{
		this->Prefix=Prefix;
		this->Stem=Stem;
		this->Suffix=Suffix;
	}

};
#endif

class Stemmer {
public://protected:
	multiply_params multi_p;
	item_types type;
	bool get_all_details;
	//for use in on_match()
	QVector<minimal_item_info> * prefix_infos;
	minimal_item_info * stem_info;
	QVector<minimal_item_info> * suffix_infos;

	QStringRef getString() {return info.text->midRef(info.start,info.finish-info.start+1);}
#ifdef RUNON_WORDS
private:
	int runwordIndex;
	QList<SubMachines> machines;
	friend class SuffixMachine;
private:
	void removeLastMachines();
#endif
public:
	bool called_everything;
	PrefixMachine* Prefix;
	StemMachine* Stem;
	SuffixMachine* Suffix;
	text_info info;

	Stemmer(QString *text,int start,bool get_all_details=true) {
		prefix_infos=NULL;
		stem_info=NULL;
		suffix_infos=NULL;
		info.start=start;
		info.finish=start;
		info.text=text;
		this->get_all_details=get_all_details;
		Prefix=new PrefixMachine(this,start);
		Stem=NULL;
		Suffix=NULL;
		multi_p=M_ALL;
	#ifdef RUNON_WORDS
		runwordIndex=0;
		if (!sarfParameters.enableRunonwords)
			machines.append(SubMachines(NULL, NULL, NULL)); //just to fill it with anything so that in on_match machine.size()==1
	#endif
	}
	void setSolutionSettings(multiply_params params) {
		multi_p=params;
		Prefix->setSolutionSettings(params);
	}
	bool operator()()//if returns true means there was a match
	{
		called_everything=true;
		return Prefix->operator ()();
	}
	bool operator()(item_types type)//used for detecting all word parts that start at this position; if returns true means there was a match
	{
		called_everything=false;
		this->type=type;
		//total_matches_till_now=0;
		if (type==PREFIX)
			return Prefix->operator ()();
		else if (type==STEM)
			return Stem->operator ()();
		else if (type==SUFFIX)
			return Suffix->operator ()();
		else
			return false;
	}
	bool on_match_helper();
	virtual bool on_match();
	virtual ~Stemmer()
	{
		if (Suffix)
			delete Suffix;
		if (Stem)
			delete Stem;
		if (Prefix)
			delete Prefix;
	}
};
#if 0
inline void printMachines(QList<SubMachines> m)
{
	qDebug()<<"[";
	for (int i=0;i<m.count();i++)
		qDebug()<<(long)m[i].Prefix<<","<<(long)m[i].Stem<<","<<(long)m[i].Suffix;
	qDebug()<<"]";
}
#else
inline void printMachines(QList<SubMachines> ){}
#endif

#endif	/* _STEMMER_H */

