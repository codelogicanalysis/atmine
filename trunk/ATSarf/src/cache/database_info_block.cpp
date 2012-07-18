#include <QFile>
#include <QDateTime>
#include <QDataStream>
#include "database_info_block.h"
#include "atmTrie.h"
#include "common.h"
#include "sql_queries.h"
#include "Search_by_item.h"
#include "ATMProgressIFC.h"
#include "Retrieve_Template.h"
#include <assert.h>
#include <QDebug>

#ifdef USE_TRIE

#ifdef LOAD_FROM_FILE
#ifdef REDUCE_THRU_DIACRITICS
inline QString cache_version()
{
	return "RD";
}
#else
inline QString cache_version()
{
	return "ND";
}
#endif
#endif

void database_info_block::readTrieFromDatabaseAndBuildFile()
{
        QSqlQuery query(theSarf->db);
#ifdef REDUCE_THRU_DIACRITICS
	QString stmt=QString("SELECT stem.id, stem.name, stem_category.category_id, stem_category.raw_data FROM stem, stem_category WHERE stem.id=stem_category.stem_id ORDER BY stem.id ASC");
#else
	QString stmt=QString("SELECT stem.id, stem.name, stem_category.category_id FROM stem, stem_category WHERE stem.id=stem_category.stem_id ORDER BY stem.id ASC");
#endif
	QString name,raw_data;
	long category_id;
	long long  stem_id, last_id;
	if (!execute_query(stmt,query))
		return;
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
	StemNode * node = NULL;
	int index=0;
	last_id=-1;
	int total=query.size(), current=0;
        if(prgsIFC != NULL)
            prgsIFC->setCurrentAction("STEM TRIE");
	while (query.next())
	{
		stem_id=query.value(0).toULongLong();
		if (last_id!=stem_id)
		{
			if (last_id!=-1)
			{
				database_info.trie_nodes->insert(index,*node);
				delete node;
				database_info.Stem_Trie->store(name,index);
				index++;
			}
			last_id=stem_id;
			node=new StemNode();
			node->key=name;
			node->stem_id=stem_id;
		}
		name=query.value(1).toString();
		category_id=query.value(2).toLongLong();
#ifdef REDUCE_THRU_DIACRITICS
		raw_data=query.value(3).toString();
		if(!equal(name,raw_data)) {
			error<<"Conflict Database:\t"<<name<<"\t"<<raw_data;
			if (raw_data.endsWith(' ')) {
                                while (raw_data.endsWith(' '))
                                    raw_data=raw_data.remove(raw_data.size()-1,1);
				assert(equal(raw_data,name));
			} else {
				name=removeDiacritics(name);
			}

		}
		node->add_info(category_id,raw_data);
#else
		node->add_info(category_id);
#endif
		current++;
                if(prgsIFC != NULL)
                    prgsIFC->report((double)current/total*100+0.5);
	}
#ifdef LOAD_FROM_FILE
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
	database_info.Stem_Trie->save(trie_path.toStdString().data());
	QFile file(trie_list_path.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out << cache_version();
		out << *(database_info.trie_nodes);
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write TRIE to file\n";
#endif
}

void database_info_block::buildTrie()
{
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
#ifndef LOAD_FROM_FILE
	readTrieFromDatabaseAndBuildFile();
#else
    QFile file(trie_list_path);
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		QString version;
		in >> version;
		if (version==cache_version()) {
			in >> *(database_info.trie_nodes);
			file.close();
		} else {
			file.close();
			readTrieFromDatabaseAndBuildFile();
			return;
		}

		QFile input(trie_path);

		if (input.open(QIODevice::ReadOnly)) {

            if (database_info.Stem_Trie != NULL)
                delete database_info.Stem_Trie;

			input.close();
                        //const char * fPath = trie_path.toLatin1();
                        QString fPath = trie_path;
                        try {
                            database_info.Stem_Trie=new ATTrie(fPath.toLatin1() );
                        }  catch(const char * ex) {
                            error<<"Fail to build stem trie from file "<<fPath<<". Exception is "<<ex<<'.'<<endl;
                        }
			//out<<QDateTime::currentDateTime().time().toString()<<"\n";
		}
	}
	else
		readTrieFromDatabaseAndBuildFile();
#endif
}
#endif

void database_info_block::fillMap(item_types type,ItemCatRaw2AbsDescPosMap * map)
{
        QSqlQuery query(theSarf->db);
	QString table = interpret_type(type);
        if(prgsIFC != NULL)
            prgsIFC->setCurrentAction(table.toUpper()+" INFO");
	for (int i=0;i<(type==STEM?1:2);i++) {
		QString stmt( "SELECT %1_id, category_id, raw_data, POS, description_id, abstract_categories FROM %1_category");
		stmt=stmt.arg(table).append((type==STEM?"":QString(" WHERE reverse_description=%1").arg(i)));
		assert (execute_query(stmt,query));
		int size=query.size();
		int counter=0;
		while (query.next()) {
			long long item_id=query.value(0).toULongLong();
			long category_id =query.value(1).toULongLong();
			QString raw_data=query.value(2).toString();
			QString POS=query.value(3).toString();
			long description_id=query.value(4).toULongLong();
			dbitvec abstract_categories(max_sources);
			abstract_categories=string_to_bitset(query.value(5));
			if (type!=STEM) {
				if (i==1)
					setReverseDirection(abstract_categories);
				else
					resetReverseDirection(abstract_categories);
			}
			ItemEntryKey key(item_id,category_id,raw_data);
			ItemEntryInfo entry(abstract_categories,description_id,POS);
			map->insertMulti(key,entry);
			counter++;
                        if(prgsIFC != NULL)
                            prgsIFC->report((double)counter/size*100+0.5);
		}
	}
#ifdef LOAD_FROM_FILE
	QString fileName;
	if (type==PREFIX)
		fileName=prefix_info_path;
	else if (type==STEM)
		fileName=stem_info_path;
	else if (type==SUFFIX)
		fileName=suffix_info_path;
	QFile file(fileName.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out<< *(map);
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write DESCRIPTION to file\n";
#endif
}

void database_info_block::buildMap(item_types type,ItemCatRaw2AbsDescPosMap * map)
{
#ifndef LOAD_FROM_FILE
	fillMap(type,map);
#else
	QString fileName;
	if (type==PREFIX)
		fileName=prefix_info_path;
	else if (type==STEM)
		fileName=stem_info_path;
	else if (type==SUFFIX)
		fileName=suffix_info_path;
	QFile file(fileName.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in>> *(map);
		file.close();
	}
	else
		fillMap(type,map);
#endif
}

database_info_block::database_info_block()
{
    Prefix_Tree=new tree();
    Suffix_Tree=new tree();
#ifdef USE_TRIE
    try {
	Stem_Trie= new ATTrie();
    }catch(const char * ex)  {
        error<<"Fail to build step trie. Exception is "<<ex<<'.'<<endl;
    }
	trie_nodes=new StemNodesList();
#endif
	comp_rules=new compatibility_rules();

	map_prefix=new ItemCatRaw2AbsDescPosMap;
	map_stem=new ItemCatRaw2AbsDescPosMap;
	map_suffix=new ItemCatRaw2AbsDescPosMap;

	descriptions=new QVector<QString>;
}

void database_info_block::readDescriptionsFromDatabaseAndBuildFile()
{
    if(prgsIFC != NULL)
        prgsIFC->setCurrentAction("DESCRIPTIONS");
    int size=0;
    {//get max_id
        Retrieve_Template max_id("description","max(id)","");
        if (max_id.retrieve())
            size=max_id.get(0).toInt()+1;
    }
    delete descriptions;
    descriptions=new QVector<QString>(size);
    Retrieve_Template desc("description","id","name","");
    int row=0, id=0;
    if (desc.retrieve())
	{
		while(row<size) //just in case some ID's are not there we fill them invalid
		{
			if (row==id+1)
				assert (desc.retrieve());
			id=desc.get(0).toLongLong();
			if (row==id)
				(*descriptions)[row]=desc.get(1).toString();
			else
				(*descriptions)[row]="";
			row++;
                        if(prgsIFC != NULL)
                            prgsIFC->report((double)row/size*100+0.5);
		}
	}
#ifdef LOAD_FROM_FILE
	QFile file(description_path.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out<< *(descriptions);
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write DESCRIPTION to file\n";
#endif
}
void database_info_block::buildDescriptions()
{
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
#ifndef LOAD_FROM_FILE
	readDescriptionsFromDatabaseAndBuildFile();
#else
	QFile file(description_path.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in>> *(descriptions);
		file.close();
	}
	else
		readDescriptionsFromDatabaseAndBuildFile();
#endif
}

void database_info_block::fill(ATMProgressIFC *p)
{
	prgsIFC=p;
	Prefix_Tree->build_affix_tree_from_file(PREFIX);
	Suffix_Tree->build_affix_tree_from_file(SUFFIX);
#ifdef USE_TRIE
	buildTrie();
#endif
	comp_rules->buildFromFile();

	buildDescriptions();

	buildMap(PREFIX,map_prefix);
	buildMap(STEM,map_stem);
	buildMap(SUFFIX,map_suffix);
        p->resetActionDisplay();
	filling=false;
}

database_info_block::~database_info_block()
{
    delete Prefix_Tree;
    delete Suffix_Tree;
#ifdef USE_TRIE
	delete Stem_Trie;
	delete trie_nodes;
#endif
	delete comp_rules;

	delete map_stem;
	delete map_prefix;
	delete map_suffix;
	delete descriptions;
}

database_info_block database_info;
