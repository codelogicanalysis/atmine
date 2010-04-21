#include "Search_by_item.h"
#include "sql_queries.h"

bool Search_by_item::retrieve_internal(long &category_id) //returns just a category but can contain redundancy
{
        bool ok;
        category_id =query.value(1).toULongLong(&ok); //1=category_id
        if (!ok)
        {
                error << "Unexpected Error: Non-integer category_id's\n";
                return false;
        }
        return true;
}
bool Search_by_item::retrieve_internal(all_item_info &info)
{
//#define toLL() toULongLong(&ok); if (!ok) { error << "Unexpected Error: Non-integer field\n"; return false; }

		info.item_id=query.value(0).toULongLong();
		info.category_id =query.value(1).toULongLong();
        info.sources=string_to_bitset(query.value(2));
        info.raw_data=query.value(3).toString();
        info.POS=query.value(4).toString();
        if (query.value(5).isNull())
                info.description="";
        else
        {
				long long description_id=query.value(5).toULongLong();
				if (description_id<0)
					info.description="";
				else
					info.description=getColumn("description","name",description_id);  //uses global query
        }
        if (type==STEM)
        {
                info.abstract_categories=string_to_bitset(query.value(6));
                info.lemma_ID=query.value(7).toString();
        }
        else
        {
                info.abstract_categories.reset();
                info.lemma_ID="";
        }
        return true;
}
bool Search_by_item::retrieve_internal(minimal_item_info &minimal)
{
        all_item_info all;
        if (!retrieve_internal(all))
                return false;
        minimal.abstract_categories=all.abstract_categories;
        minimal.category_id=all.category_id;
        minimal.description=all.description;
        minimal.POS=all.POS;
        minimal.raw_data=all.raw_data;
        return true;
}
/*bool Search_by_item::execute_query(QString stmt) //just a copy of the global one, bc we need to use the local query and not the global
{
        if (!query.exec(stmt))
        {
                error <<query.lastError().text()<<"\n";
                return false;
        }
        return true;
}*/
Search_by_item::Search_by_item(item_types type,long long id)
{
        QSqlQuery temp(db);
        query=temp;
        this->type=type;
        this->id=id;
        QString table = interpret_type(type);
        if (id!=-1)
        {
                name=getColumn(table,"name",id); //will use the global query
                //QString stmt( "SELECT DISTINCT category_id FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
                QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
                stmt=stmt.arg(table).arg(id).arg((type==STEM?", abstract_categories, lemma_ID":""));
                if (!execute_query(stmt,query)) //will use the local query
                        id=-1; //not really, but because an error took place
        }
}
Search_by_item::Search_by_item(item_types type,QString name)
{
        QSqlQuery temp(db);
        query=temp;
        this->type=type;
        this->name=name;
        QString table = interpret_type(type);
        //maybe better here get information about GrammarStem, and its sources and save it, but not a problem, or not??
        id=getID(table,name); //will use the global query
        if (id!=-1)
        {
                //QString stmt( "SELECT DISTINCT category_id FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
                QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category WHERE %1_id ='%2' ORDER BY category_id ASC");
                stmt=stmt.arg(table).arg(id).arg((type==STEM?", abstract_categories, lemma_ID":""));
                if (!execute_query(stmt,query)) //will use the local query
                        id=-1; //not really, but because an error took place
        }
}
Search_by_item::Search_by_item(item_types type,QVector<QString> names)
{
		QSqlQuery temp(db);
		query=temp;
		this->type=type;
		QString table = interpret_type(type);
		//maybe better here get information about GrammarStem, and its sources and save it, but not a problem, or not??
		QString where;
		int count=0;
		for (int i=0;i<names.count();i++)
		{
			int temp_id=getID(table,names[i]); //will use the global query
			if (temp_id!=-1)
			{
				id=temp_id;
				this->name=names[i];
				if (count!=0)
					where.append(" OR ");
				else
					where.append("WHERE ");
				where.append(QString("%1_id ='%2'").arg(table).arg(id));
			}
		}
		if (count==0)
			id=-1;
		else
		{
			QString stmt( "SELECT %1_id, category_id, sources, raw_data, POS, description_id %3 FROM %1_category %2 ORDER BY category_id ASC");
			stmt=stmt.arg(table).arg(where).arg((type==STEM?", abstract_categories, lemma_ID":""));
			if (!execute_query(stmt,query)) //will use the local query
				id=-1; //not really, but because an error took place
		}
}
long long Search_by_item::ID()
{
        return id;
}
QString Search_by_item::Name()
{
        if (id!=-1)
                return name;
        else
                return QString::null;
}
int Search_by_item::size() //total size and not what is left
{
        return query.size();
}
bool Search_by_item::retrieve(long &category_id) //returns just a category but can contain redundancy
{
        if (id !=-1 && query.next())
                return retrieve_internal(category_id);
        else
                return false;
}
bool Search_by_item::retrieve(all_item_info & info)
{
        if (id !=-1 && query.next())
                return retrieve_internal(info);
        else
                return false;
}
bool Search_by_item::retrieve(minimal_item_info &info)
{
        if (id !=-1 && query.next())
                return retrieve_internal(info);
        else
                return false;
}
int Search_by_item::retrieve(long category_ids[],int size_of_array)
{
        if (size_of_array<=0 || id ==-1)
                return 0;
        int i;
        for (i=0; i<size_of_array && query.next();i++)
        {
                if (!retrieve_internal(category_ids[i]))
                        return -1;
        }
        return i;
}
#define retrieve_info()  \
{\
        if (size_of_array<=0 || id ==-1) \
                        return 0; \
        int i; \
        for (i=0; i<size_of_array && query.next();i++) \
        { \
                if (!retrieve_internal(info[i])) \
                        return -1; \
        } \
        return i; \
}
int Search_by_item::retrieve(minimal_item_info info[], int size_of_array)
{
        retrieve_info();
}
int Search_by_item::retrieve(all_item_info info[], int size_of_array)
{
        retrieve_info();
}
QString Search_by_item::getGrammarStem() //just for stems
{
        if (type==STEM)
        {
                bool ok;
                long long grammar_stem_id= getColumn("stem","grammar_stem_id",id).toULongLong(&ok);
                if (!ok)
                {
                        error << "Non-integer Grammar Stem ID\n";
                        return "";
                }
                return getColumn("stem","name",grammar_stem_id);
        }
        else
        {
                warning << "getGrammarStem() does not work for PREFIX or SUFFIX\n";
                return "";
        }
}
bitset<max_sources> Search_by_item::getGrammarStem_sources() //just for stems
{
        if (type==STEM)
                return getSources("stem",id);
        else
        {
                warning << "getGrammarStem_sources() does not work for PREFIX or SUFFIX\n";
                return INVALID_BITSET;
        }
}
