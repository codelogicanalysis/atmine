#ifndef _STEMMER_H
#define	_STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "common.h"
#include <QVector>

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

class Stemmer
{
public://protected:
	multiply_params multi_p;
	item_types type;
	bool get_all_details;
	//for use in on_match()
	QVector<minimal_item_info> * prefix_infos;
	minimal_item_info * stem_info;
	QVector<minimal_item_info> * suffix_infos;
public:
	bool called_everything;
	PrefixMachine* Prefix;
	StemMachine* Stem;
	SuffixMachine* Suffix;
	text_info info;

	Stemmer(QString *text,int start,bool get_all_details=true)
	{
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
	}
	void setSolutionSettings(multiply_params params)
	{
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
	virtual bool on_match_helper();
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

#endif	/* _STEMMER_H */

