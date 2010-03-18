#ifndef _RESULT_NODE_H
#define	_RESULT_NODE_H

class result_node:public node
{
    private:
        long previous_category_id;
        long affix_id;
        long resulting_category_id;
    public:

    #ifdef REDUCE_THRU_DIACRITICS
        QList<QString> raw_datas;

        void add_raw_data(QString raw_data);
    #endif

    #ifdef MEMORY_EXHAUSTIVE
        //TODO: change Pair to a public class outside result_node
        class Additional
        {
            QString raw_data;
            QString description;
            QString
        } ;
        typedef QList<StringTriple> StringTriples;
        StringPairs rawdata_description; //made public to reduce copying of strings

        void addPair(QString raw_data,QString description);
    #endif
        result_node(long affix_id,long previous_category_id,long resulting_category_id, QString raw_data, QString description);
        bool isLetterNode();
        long get_previous_category_id();
        void set_previous_category_id(long id);
        long get_affix_id();
        void set_affix_id(long id);
        long get_resulting_category_id();
        void set_resulting_category_id(long id);
        QString to_string(bool isAffix=true);
        ~result_node(){	}
};

#endif	/* _RESULT_NODE_H */

