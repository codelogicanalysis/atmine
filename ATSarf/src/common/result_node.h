#ifndef _RESULT_NODE_H
#define	_RESULT_NODE_H

#include "node.h"
#include "sql_queries.h"

class RawData {
private:
	QString original;
	QString inflected;
public:
	QString getActual() {
		if (inflected=="*")
			return original;
		else
			return inflected;
	}
	QString getOriginal() {
		return original;
	}
	RawData(QString original, QString inflected="*") {
		this->original=original;
		if (original==inflected) {
			this->inflected="*";
		} else {
			this->inflected=inflected;
		}
	}
	bool operator ==(const RawData & r) {
		return original==r.original && inflected==r.inflected;
	}
};


class result_node:public node {
    private:
        long previous_category_id;
        long affix_id;
                long resulting_category_id :63;
                bool isAcceptState:1;
                QString inflectionRule;

                void initialize(const result_node & n)
                {
                #ifdef REDUCE_THRU_DIACRITICS
                        raw_datas=n.raw_datas;
                #endif
                        set_previous_category_id(n.previous_category_id);
                        set_resulting_category_id(n.resulting_category_id);
                        set_affix_id(n.affix_id);
                        inflectionRule=n.inflectionRule;
                }
    public:

		void setInflectionRule(QString rule) {
			inflectionRule=rule;
		}
		QString getInflectionRule() {
			return inflectionRule;
		}

    #ifdef REDUCE_THRU_DIACRITICS
		QList<RawData> raw_datas;
		void add_raw_data(QString original, QString inflected)
		{
			RawData r(original,inflected);
			if (!raw_datas.contains(r))
				this->raw_datas.append(r);
		}
    #endif

	#ifdef REDUCE_THRU_DIACRITICS
		result_node(long affix_id,long previous_category_id,long resulting_category_id,bool isAccept,QString raw_data, QString inflected):node()
		{
			this->raw_datas.clear();
			add_raw_data(raw_data, inflected);
	#else
		result_node(long affix_id,long previous_category_id,long resulting_category_id,isAccept)
		{
	#endif
			set_previous_category_id(previous_category_id);
			set_resulting_category_id(resulting_category_id);
			set_affix_id(affix_id);
			set_accept_state(isAccept);
		}
		result_node(const result_node & n):node(n)
		{
			initialize(n);
		}
		result_node& operator=(const result_node& n)
		{
			initialize(n);
			return operator =(n);
		}
		bool isLetterNode() const
		{
			return false;
		}
		long get_previous_category_id()
		{
			return previous_category_id;
		}
		void set_previous_category_id(long id)
		{
			previous_category_id=id;
		}
		long get_affix_id()
		{
			return affix_id;
		}
		void set_affix_id(long id)
		{
			affix_id=id;
		}
		long get_resulting_category_id()
		{
			return resulting_category_id;
		}
		void set_resulting_category_id(long id)
		{
			resulting_category_id=id;
		}
		void set_accept_state(bool val) {
			isAcceptState=val;
		}
		bool is_accept_state() {
			return isAcceptState;
		}
		QString to_string(bool isAffix=true)
		{
			return QString("-").append(QString("%1").arg((isAffix?
														  getColumn("category","name",previous_category_id):
														  QString("%1").arg(previous_category_id)))).append(">[").append(QString("%1").arg((isAffix?
																																			getColumn("category","name",resulting_category_id):
																																			QString("%1").arg(resulting_category_id)))).append("]");
		}
		result_node * getPreviousResultNode()
		{
			//TODO: check memory corruption
			node * previous=parent;
			while (previous!=NULL){
								  if (!previous->isLetterNode()) break;
								  previous=previous->parent;}
			return (result_node*)previous;
		}
		~result_node(){}
};

#endif	/* _RESULT_NODE_H */

