#ifndef _RESULT_NODE_H
#define	_RESULT_NODE_H

#include "node.h"
#include "sql_queries.h"

class result_node:public node
{
    private:
        long previous_category_id;
        long affix_id;
        long resulting_category_id;

		void initialize(const result_node & n)
		{
			#ifdef REDUCE_THRU_DIACRITICS
					raw_datas=n.raw_datas;
			#endif
			set_previous_category_id(n.previous_category_id);
			set_resulting_category_id(n.resulting_category_id);
			set_affix_id(n.affix_id);
		}
    public:

    #ifdef REDUCE_THRU_DIACRITICS
        QList<QString> raw_datas;
		void add_raw_data(QString raw_data)
		{
			if (!raw_datas.contains(raw_data))
				this->raw_datas.append(raw_data);
		}
    #endif

	#ifdef REDUCE_THRU_DIACRITICS
		result_node(long affix_id,long previous_category_id,long resulting_category_id,QString raw_data):node()
		{
			this->raw_datas.clear();
			add_raw_data(raw_data);
	#else
		result_node(long affix_id,long previous_category_id,long resulting_category_id)
		{
	#endif
			set_previous_category_id(previous_category_id);
			set_resulting_category_id(resulting_category_id);
			set_affix_id(affix_id);
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
		QString to_string(bool isAffix=true)
		{
			return QString("-").append(QString("%1").arg((isAffix?getColumn("category","name",previous_category_id):QString("%1").arg(previous_category_id)))).append(">[").append(QString("%1").arg((isAffix?getColumn("category","name",resulting_category_id):QString("%1").arg(resulting_category_id)))).append("]");
		}
		result_node * getPreviousResultNode()
		{
			node * previous=parent;
			while (previous!=NULL  && previous->isLetterNode())
				previous=previous->parent;
			return (result_node*)previous;
		}
		~result_node(){}
};

#endif	/* _RESULT_NODE_H */

