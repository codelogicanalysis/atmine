#ifndef INSERT_SCRIPTS_H
#define INSERT_SCRIPTS_H

#include "sql-interface.h"
#include "trie.h"
#include <QDir>
#include <QStringList>

int insert_buckwalter()
{
	int source_id=insert_source("Buckwalter Dictionaries","modifying aramorph.pl + insert_buckwalter() c++ code fragment","Jad Makhlouta");
	//items
	const QString item_files[3]= {"../../buckwalter scripts/list_of_stems.txt", \
								  "../../buckwalter scripts/list_of_prefixes.txt", \
								  "../../buckwalter scripts/list_of_suffixes.txt"};
	const item_types types[3] ={ STEM, PREFIX, SUFFIX};
	for (int j=0;j<3;j++)
	{
		int num_entries=6;
		QFile input(item_files[j]);
		if (!input.open(QIODevice::ReadWrite))
		{
			out << "File not found\n";
			return 1;
		}
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
				//out<<"Double quotations in "<<description<<" replaced by single quotations.\n";
				description.replace("\"","'"); //instead we will face some problems in insert...
			}
			//3 letters are not properly transfered using perl script because it is not based on unicode, here we solve them manually
			if (item.contains("{") || item.contains("`") || item.contains("V"))
			{
				QString before=item;
				item=item.replace("{",QString(QChar(0x0671)));
				item=item.replace("`",QString(QChar(0x0670)));
				item=item.replace("V",QString(QChar(0x06A4)));
				//out << "Replaced "<<interpret_type(types[j])<<": "<<before<<" by "<<item<<"\n";
			}
			if (raw_data.contains("{") || raw_data.contains("`") || raw_data.contains("V"))
			{
				QString before=raw_data;
				raw_data=raw_data.replace("{",QString(QChar(0x0671)));
				raw_data=raw_data.replace("`",QString(QChar(0x0670)));
				raw_data=raw_data.replace("V",QString(QChar(0x06A4)));
				//out << "Replaced "<<interpret_type(types[j])<<": "<<before<<" by "<<raw_data<<"\n";
			}
			QString abstract_category;
			long abstract_id=-1;
			if (types[j]==STEM)
			{
				//if (!POS.contains("+"))//choose what to do with POS like "litaw~i/ADV+hi/PRON_3MS"
				{
					QStringList abstract_list=POS.split("+").at(0).split("/");
					if (abstract_list.size()>=2)
						abstract_category=	abstract_list.at(1);
					else
					{
						abstract_category="";
						out << "Unexpected Error: split on "<<POS<<" "<<abstract_list.size()<<" "<<abstract_list.at(0);
						return 1;
					}
					if ((abstract_id=getID("category",abstract_category,QString("type=%1 AND abstract=1").arg((int)STEM)))==-1)
					{
						abstract_id=insert_category(abstract_category,STEM,source_id,true);
						out<< QString("Inserted new Abstract Category '%1'\n").arg(abstract_category);
					}
				}
			}
			if ((types[j]==STEM?insert_item(STEM,item,raw_data,category,source_id,abstract_id,description,POS,"",lemmaID)<0:insert_item(types[j],item,raw_data,category,source_id,-1,description,POS,"","")<0))
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(interpret_type(types[j]));
		input.close();
	}
	//compatibility rules
	const QString rules_files[3]= {"../../buckwalter scripts/tableAB", \
								  "../../buckwalter scripts/tableBC", \
								  "../../buckwalter scripts/tableAC"};
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
	return 0;
}

int insert_propernames()
{
	QDir folder("../../../dic/N","*.txt");
	if (!folder.exists())
	{
		out << "Invalid Folder\n";
		return -1;
	}
	long abstract_Noun_Prop_id=getID("category","NOUN_PROP",QString("abstract=1 AND type=%1").arg((int)STEM));
	long category_NProp_id=getID("category","Nprop",QString("abstract=0 AND type=%1").arg((int)STEM));
	QString file_name;
	long abstract_category_id;
	foreach (file_name,folder.entryList())
	{
		if (file_name=="all_n.txt")
		{
			out << "Ignored all_n.txt file\n";
			continue;
		}
		QFile input(folder.absolutePath().append(QString("/").append(file_name)));
		if (!input.open(QIODevice::ReadWrite))
		{
			out << "Unexpected Error: File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		int line_num=0;
		int source_id;
		QString source;
		QString normalization_process;
		while (!file.atEnd())
		{
			line_num++;
			QString line=file.readLine(0);
			if (line_num==1)
			{
				source=line;
				continue;
			}
			else if (line_num==2)
			{
				normalization_process=line;
				continue;
			}
			else if (line_num==3)
			{
				source_id=insert_source(source,normalization_process,line);
				abstract_category_id=insert_category(file_name.split(".").at(0),STEM,source_id,true);
				continue;
			}
			if (line.isNull())
			{
				line_num--; //finished
				break;
			}
			if (line.isEmpty()) //ignore empty lines if they exist
				continue;
			int stem_id=insert_item(STEM,line,line,"Nprop",source_id,abstract_category_id,"Proper noun","","","");
			if (stem_id<0)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
			if (addAbstractCategory("stem_category",abstract_Noun_Prop_id,-1,QString("stem_id=%1 AND category_id=%2").arg(stem_id).arg(category_NProp_id),false)==NULL)
			{
				out << "Unexpected Error: Unable to add Abstract Category Noun_Prop to stem_id="<<stem_id<<"\n";
				return -1;
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name);
		input.close();
	}
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
	if (insert_buckwalter()<0)/*generate_all_prefixes()*/
		return -1;
	if (insert_propernames()<0)
		return -1;
	return 0;
}

#endif // INSERT_SCRIPTS_H
