#include "result_node.h"

#include "../sql-interface/sql_queries.h"

#ifdef REDUCE_THRU_DIACRITICS
    void result_node::add_raw_data(QString raw_data)
    {
        if (!raw_datas.contains(raw_data))
            this->raw_datas.append(raw_data);
    }
#endif

#ifdef MEMORY_EXHAUSTIVE
    void result_node::addPair(QString raw_data,QString description)
    {
        if (!rawdata_description.contains(StringPair(raw_data,description)))
            rawdata_description.append(StringPair(raw_data,description));
    }

    result_node::result_node(long affix_id,long previous_category_id,long resulting_category_id, QString raw_data, QString description)
    {
        rawdata_description.clear();
        addPair(raw_data,description);
#elif defined (REDUCE_THRU_DIACRITICS)
	result_node::result_node(long affix_id,long previous_category_id,long resulting_category_id,QString raw_data):node()
    {
        this->raw_datas.clear();
        add_raw_data(raw_data);
#else
    result_node::result_node(long affix_id,long previous_category_id,long resulting_category_id)
    {
#endif
        set_previous_category_id(previous_category_id);
        set_resulting_category_id(resulting_category_id);
        set_affix_id(affix_id);
    }
	void result_node::initialize(const result_node & n)
	{
		#ifdef REDUCE_THRU_DIACRITICS
				raw_datas=n.raw_datas;
		#endif
		set_previous_category_id(n.previous_category_id);
		set_resulting_category_id(n.resulting_category_id);
		set_affix_id(n.affix_id);
	}
	result_node::result_node(const result_node & n):node(n)
	{
		initialize(n);
	}
	result_node& result_node::operator=(const result_node& n)
	{
		initialize(n);
		return operator =(n);
	}
	bool result_node::isLetterNode() const
    {
        return false;
    }
    long result_node::get_previous_category_id()
    {
        return previous_category_id;
    }
    void result_node::set_previous_category_id(long id)
    {
        previous_category_id=id;
    }
    long result_node::get_affix_id()
    {
        return affix_id;
    }
    void result_node::set_affix_id(long id)
    {
        affix_id=id;
    }
    long result_node::get_resulting_category_id()
    {
        return resulting_category_id;
    }
    void result_node::set_resulting_category_id(long id)
    {
        resulting_category_id=id;
    }
	QString result_node::to_string(bool isAffix)
    {
        return QString("-").append(QString("%1").arg((isAffix?getColumn("category","name",previous_category_id):QString("%1").arg(previous_category_id)))).append(">[").append(QString("%1").arg((isAffix?getColumn("category","name",resulting_category_id):QString("%1").arg(resulting_category_id)))).append("]");
    }
    result_node::~result_node(){	}


