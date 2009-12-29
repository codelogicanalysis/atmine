#ifndef INSERT_SCRIPTS_H
#define INSERT_SCRIPTS_H

#include "sql-interface.h"
int insert_buckwalter()
{
	int source_id=insert_source("Buckwalter Dictionaries","modifying aramorph.pl + insert_buckwalter() c++ code fragmant","Jad Makhlouta");
	//items
	const QString item_files[3]= {"/home/jad/Desktop/tools/trunk/src/buckwalter scripts/list_of_stems.txt", \
								  "/home/jad/Desktop/tools/trunk/src/buckwalter scripts/list_of_prefixes.txt", \
								  "/home/jad/Desktop/tools/trunk/src/buckwalter scripts/list_of_suffixes.txt"};
	const item_types types[3] ={ STEM, PREFIX, SUFFIX};
	for (int j=0;j<3;j++)
	{
		int num_entries=6;
		QFile input(item_files[j]);
		if (!input.open(QIODevice::ReadWrite))
			return 1;
		QTextStream file(&input);
		file.setCodec("utf-8");
		int line_num=0;
		while (!file.atEnd())
		{
			line_num++;
			QString line=file.readLine(0);
			if (line.isNull())
			{
				line_num--; //finished
				break;
			}
			if (line.isEmpty()) //ignore empty lines if they exist
				continue;
			QStringList entries=line.split("\t",QString::KeepEmptyParts);
			if (entries.size()!=num_entries)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
			QString item=entries[0];
			QString raw_data=entries[1];
			QString category=entries[2];
			QString description=entries[3];
			QString POS=entries[4];
			QString lemmaID=entries[5]; //"" for non-STEMS but ignored later
			if (description.contains("\""))
			{
				out<<"Double quotations in "<<description<<" replaced by single quotations.\n";
				description.replace("\"","'"); //instead we will face some problems in insert...
			}
			//3 letters are not properly transfered using perl script because it is not based on unicode, here we solve them manually
			if (item.contains("{") || item.contains("`") || item.contains("V"))
			{
				QString before=item;
				item=item.replace("{",QString(QChar(0x0671)));
				item=item.replace("`",QString(QChar(0x0670)));
				item=item.replace("V",QString(QChar(0x06A4)));
				out << "Replaced "<<interpret_type(types[j])<<": "<<before<<" by "<<item<<"\n";
			}
			if (raw_data.contains("{") || raw_data.contains("`") || raw_data.contains("V"))
			{
				QString before=raw_data;
				raw_data=raw_data.replace("{",QString(QChar(0x0671)));
				raw_data=raw_data.replace("`",QString(QChar(0x0670)));
				raw_data=raw_data.replace("V",QString(QChar(0x06A4)));
				out << "Replaced "<<interpret_type(types[j])<<": "<<before<<" by "<<raw_data<<"\n";
			}
			if ((types[j]==STEM?insert_item(STEM,item,raw_data,category,source_id,-1,description,POS,"",lemmaID)<0:insert_item(types[j],item,raw_data,category,source_id,-1,description,POS,"","")<0))
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(interpret_type(types[j]));
		input.close();
	}
	//compatibility rules
	const QString rules_files[3]= {"/home/jad/Desktop/tools/trunk/src/buckwalter scripts/tableAB", \
								  "/home/jad/Desktop/tools/trunk/src/buckwalter scripts/tableBC", \
								  "/home/jad/Desktop/tools/trunk/src/buckwalter scripts/tableAC"};
	const rules rule[3] ={ AB, BC, AC};
	for (int j=0;j<3;j++)
	{
		QFile input(rules_files[j]);
		if (!input.open(QIODevice::ReadWrite))
			return 1;
		QTextStream file(&input);
		file.setCodec("utf-8");
		int line_num=0;
		while (!file.atEnd())
		{
			line_num++;
			QString line=file.readLine(0);
			if (line.isNull())
			{
				line_num--; //finished
				break;
			}
			if (line.isEmpty() || line.startsWith(";")) //ignore empty lines and comments if they exist
				continue;
			QStringList entries=line.split(" ",QString::KeepEmptyParts);
			if (entries.size()!=2)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
			QString cat1=entries[0];
			QString cat2=entries[1];
			if (insert_compatibility_rules(rule[j],cat1,cat2,source_id)<0)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				continue; //files contain undefined categories, so just ignore these
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 compatibility rules\n").arg(line_num).arg(interpret_type(rule[j]));
		input.close();
	}
	//3 letters are not properly transfered using perl script because it is not based on unicode, here we solve them manually
	/*const item_types item_type[3]={PREFIX,STEM,SUFFIX};
	qDebug() <<QString(QChar(0x0626));//0x0671 was documented but seems not appropriate
	qDebug() <<QString(QChar(0x0670));
	qDebug() <<QString(QChar(0x06A4));
	for (int i=0;i<3;i++)
	{
		QString stmt=QString("SELECT id,name from %1 WHERE name LIKE \"%{%\" OR name LIKE \"%`%\" OR name LIKE \"%V%\"").arg(interpret_type(item_type[i]));
		perform_query(stmt);
		while (query.next())
		{
			QString before=query.value(1).toString();
			QString after=before.replace("{",QString(QChar(0x0626)));
			after=after.replace("`",QString(QChar(0x0670)));
			after=after.replace("V",QString(QChar(0x06A4)));
			QString stmt2= QString("UPDATE %1 SET name =\"%2\" WHERE id=%3").arg(interpret_type(item_type[i])).arg(after).arg(query.value(0).toString());
			QSqlQuery query2(db);
			if (!query2.exec(stmt2))
			{
				error << "Unable to execute "<<stmt2<<"\n";
				return -1;
			}
			else
				out << "Replaced "<<interpret_type(item_type[i])<<": "<<before<<" by "<<after<<"\n";
		}
		stmt=QString("SELECT raw_data from %1_category WHERE raw_data LIKE \"%{%\" OR raw_data LIKE \"%`%\" OR raw_data LIKE \"%V%\"").arg(interpret_type(item_type[i]));
		perform_query(stmt);
		while (query.next())
		{
			QString before=query.value(0).toString();
			QString after=before.replace("{",QString(QChar(0x0626)));
			after=after.replace("`",QString(QChar(0x0670)));
			after=after.replace("V",QString(QChar(0x06A4)));
			QString stmt2= QString("UPDATE %1_category SET raw_data =\"%2\" WHERE raw_data=\"%3\"").arg(interpret_type(item_type[i])).arg(after).arg(before);
			QSqlQuery query2(db);
			if (!query2.exec(stmt2))
			{
				error << "Unable to execute "<<stmt2<<"\n";
				return -1;
			}
			else
				out << "Replaced raw_data in"<<interpret_type(item_type[i])<<"_category: "<<before<<" by "<<after<<"\n";
		}
	}*/
	return 0;
}

int start(QString input_str, QString &output_str, QString &error_str)
{

	out.setString(&output_str);
	out.setCodec("utf-8");
	in.setString(&input_str);
	in.setCodec("utf-8");
	displayed_error.setString(&error_str);
	displayed_error.setCodec("utf-8");
	if (insert_buckwalter()<0)
		return -1;
	return 0;
}

#endif // INSERT_SCRIPTS_H
