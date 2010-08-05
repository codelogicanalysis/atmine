#ifndef _STEMMER_H
#define	_STEMMER_H

#include "prefix_search.h"
#include "stem_search.h"
#include "suffix_search.h"
#include "common.h"
#include <QVector>


class multiply_params
{
	bool raw_data:1;
	bool description:1;
	bool POS:1;
	bool abstract_category:1;
public:
	multiply_params()
	{
		raw_data=true;
		description=true;
		POS=true;
		abstract_category=true;
	}
};

static multiply_params M_ALL;

class Stemmer;
#if 0
class TreeMachine: public TreeSearch
{
public:
	Stemmer * controller;
#ifdef REDUCE_THRU_DIACRITICS
	QList<QString > raw_datasOFCurrentMatch;
#if 0
#ifdef MULTIPLICATION
	QVector<minimal_item_info> * affix_info;
#endif
protected:
#ifdef MULTIPLICATION
	multiply_params m_params;
	QList<result_node *> * result_nodes;
	void get_all_possibilities(int i);
#else
	void get_all_possibilities(int i, QList<QString> &raw_datas);
#endif
	bool a_branch_returned_false; //needed by get_all_possibilities() to stop when a false is retuned
#endif
#endif
public:
	TreeMachine(item_types type,Stemmer * controller,int start/*,multiply_params params=M_ALL */);
	virtual bool onMatch();
	virtual bool postMatch()=0;
	virtual ~TreeMachine()
	{
		/*if (affix_info!=NULL)
			delete affix_info;*/
	}
};
#endif

class PrefixMachine: public PrefixSearch//TreeMachine
{
public:
	Stemmer * controller;

	PrefixMachine(Stemmer * controller,int start);
	bool onMatch();
	bool postMatch();
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
	bool postMatch();
	virtual ~SuffixMachine(){}
};

class Stemmer
{
public://protected:
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

