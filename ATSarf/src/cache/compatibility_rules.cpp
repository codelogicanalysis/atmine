
#include "compatibility_rules.h"

#include <QString>
#include <QBitArray>
#include <QDataStream>
#include <QList>
#include <QFile>
#include <QVector>
#include <QHash>
#include <QtAlgorithms>
#include <assert.h>
#include "sql_queries.h"
#include "dbitvec.h"
#include "Retrieve_Template.h"
#include "database_info_block.h"

using namespace std;


void compatibility_rules::fill() //TODO: solve issue of abstract not read correctly
{
	int size=0;
	{//fill abstract Cat ID
		Retrieve_Template abstCatID("category","id","abstract=1");
		int i=0;
		absCatIDForBits.clear();
		absCatBitMap.clear();
		while (abstCatID.retrieve())
		{
			int id=abstCatID.get(0).toInt();
			absCatIDForBits.append(id);
			absCatBitMap.insert(id,i);
			i++;
		}
	}
	{//get max_id
		Retrieve_Template max_id("category","max(id)","");
		if (max_id.retrieve())
			size=max_id.get(0).toInt()+1;
	}
	{//initialize the double array
		database_info.prgsIFC->setCurrentAction("Compatibility Rules");
		Columns cols;
		cols.append("id");
		cols.append("type");
		cols.append("abstract");
		cols.append("name");
		Retrieve_Template category_table("category",cols,"");
		crlTable.resize(size);
		cat_names.resize(size);
		for (int i=0;i<size;i++)
		{
			crlTable[i]=QVector<comp_rule_t> (size);
			//cat_names[i].valid=false; //not needed since by default is false
		}
		int row=0, id=0;
		assert (category_table.retrieve());
		while(row<size) //just in case some ID's are not there we fill them invalid
		{
			if (row==id+1)
				assert (category_table.retrieve());
			id=category_table.get(0).toLongLong();
			item_types t;
			bool abstract;
			if (row==id)
			{
				t=(item_types)category_table.get(1).toInt();
				//abstract=category_table.get(2).toInt(); does not work
				abstract= (absCatBitMap.find(id)!=absCatBitMap.end());
			}
			else
			{
				t=ITEM_TYPES_LAST_ONE;//INVALID
				abstract=false;
			}
			cat_names[row].set(category_table.get(3).toString(),abstract!=0);
			for (int i=0;i<size;i++)
			{
				comp_rule_t & crule=crlTable[row][i];
				crule.abstract1=abstract;
				crule.typecat1=t;
				crule.valid=0;

				crule=crlTable[i][row];
				crule.abstract2=abstract;
				crule.typecat2=t;
				crule.valid=0;
			}
			row++;
			database_info.prgsIFC->report((double)row/size*50+0.5);
		}
	}
	{//fill with valid rules
		QVector<QString> cols(4);
		cols[0]="category_id1";
		cols[1]="category_id2";
		cols[2]="resulting_category";
		cols[3]="type";
		Retrieve_Template order("compatibility_rules",cols,"");
		int size=order.size();
		int i=0;
		while (order.retrieve())
		{
			unsigned int c1=order.get(0).toLongLong();
			unsigned int c2=order.get(1).toLongLong();
			unsigned int rc=order.get(2).toLongLong();
			rules rule=(rules)order.get(3).toLongLong();
			comp_rule_t & crule=crlTable[c1][c2];

			crule.valid=1;
			crule.rule_t=rule;
			if (rule==AA || rule==CC)
			{
				bool isValueNull = order.get(2).isNull();
				crule.rc= isValueNull ? c2 : rc;
			}
			item_types t1,t2;
			assert(get_types_of_rule(rule,t1,t2));
			crule.typecat1=(item_types)t1;
			crule.typecat2=(item_types)t2;
			crule.abstract1=0;
			crule.abstract2=0;
			i++;
			database_info.prgsIFC->report((double)i/size*50+50.5);
		}
	}
}

void compatibility_rules::readFromDatabaseAndBuildFile()
{
	fill();
#ifdef LOAD_FROM_FILE
	QFile file(compatibility_rules_path.toStdString().data());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< crlTable
			<< cat_names
			<< absCatIDForBits;
		file.close();
	}
	else
		error <<"Unexpected Error: Unable to write COMPATIBILITY TABLES to file\n";
#endif
}
void compatibility_rules::buildFromFile()
{
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
#ifndef LOAD_FROM_FILE
	readFromDatabaseAndBuildFile();
	fillMap();
#else
	QFile file(compatibility_rules_path.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		in	>> crlTable
			>> cat_names
			>> absCatIDForBits;
		file.close();
	}
	else
		readFromDatabaseAndBuildFile();
	fillMap();
#endif
}
void compatibility_rules::fillMap()
{
	int size=cat_names.size();
	for(int i=0;i<size;i++)
	{
		if (cat_names[i].isValid())
			catNamesMap.insert(cat_names[i].getName(),i);
	}
	size=absCatIDForBits.size();
	for(int i=0;i<size;i++)
		absCatBitMap.insert(absCatIDForBits[i],i);
}
