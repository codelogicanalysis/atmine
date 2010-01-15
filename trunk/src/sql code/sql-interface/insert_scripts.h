#ifndef INSERT_SCRIPTS_H
#define INSERT_SCRIPTS_H

#include "sql-interface.h"
#include "trie.h"
#include <QDir>
#include <QStringList>

//constantletters
const QChar ya2=QChar(0x064A);
const QChar alef=QChar(0x0627);
const QChar ta2_marbouta=QChar(0x0629);
const QChar waw=QChar(0x0648);
const QChar shadde=QChar(0x0651);
const QChar fatha=QChar(0x064E);
const QChar damma=QChar(0x064F);
const QChar kasra=QChar(0x0650);
const QChar sukun=QChar(0x0652);
const QChar lam=QChar(0x0644);

//utility functions
inline bool isConsonant(QChar letter)
{
	if (letter !=ya2 && letter !=waw && letter !=alef) //not a very firm condition to assume consonant but might work here
		return true;
	else
		return false;
}
inline bool isDiactric(QChar letter)
{
	if (letter==shadde || letter==fatha || letter==damma || letter==kasra || letter==sukun)
		return true;
	else
		return false;
}
inline QString removeDiactrics(QString /*&*/text) //in-efficient, change later
{
	/*QString changed=*/return text.remove(shadde).remove(fatha).remove(damma).remove(kasra).remove(sukun);
	/*int letters_removed=text.length()-changed.length();
	text=changed;
	return letters_removed;*/
}
inline int getLastLetter_index(QString word) //last non-diactrical letter
{
	int length=word.length();
	if (length==0)
		return -1;
	int i=length-1;
	while (i>=0 && isDiactric(word[i]))
		i--;
	return i;
}
inline QChar getLastLetter(QString word, int pos) //helper function for last non-diactric letter
{
	if (pos>=0 && pos < word.length())
		return word[pos];
	else
		return '\0';
}
inline QChar getLastLetter(QString word) //last non-diactrical letter
{
	int pos=getLastLetter_index(word);
	return getLastLetter(word,pos);
}
inline QString removeLastLetter(QString word) //last non-diactrical letter
{
	return word.left(getLastLetter_index(word));
}
inline QString removeLastDiactric(QString word) //only one Diactric is removed
{
	if (word.length()==0)
		return word;
	if (isDiactric(word[word.length()-1]))
		return word.left(word.length()-1);
	return word;
}
QString get_Possessive_form(QString word)//last letter must be computed without diactrics and when removing an unwanted letter its diactric if there must be removed... best do 2 utility functions: getLastletter, removelastLetter
{
	if (word.length()>=2)
	{
		int last_index=getLastLetter_index(word);
		QChar last=getLastLetter(word,last_index);
		QChar before=getLastLetter(word.left(last_index));
		if (last==alef && isConsonant(before))
			return removeLastDiactric(word).append(waw).append(ya2);
		else if (last==alef && before==waw )
			return removeLastDiactric(removeLastLetter(word)).append(ya2);
		else if (last==alef && before==ya2 )
			return removeLastDiactric(removeLastLetter(word)).append(shadde);
		else if (last==ta2_marbouta && isConsonant(before))
			return removeLastDiactric(removeLastLetter(word)).append(ya2);
		else if (last==ya2)
			return removeLastDiactric(word).append(shadde);
		else if (isConsonant(last) || last==waw)
			return removeLastDiactric(word).append(ya2);
		else
		{
			out << "Unknown Rule for Possessive form\n";
			return QString::null;
		}
	}
	else
		return word.append(ya2);
}
/*
alef 				+consonant before			-wey					sayda --> saydawey		or		7alba --> 7albawey
alef				+waw before					-ya2 instead of alef	nikaragwa -->nikaragwiy
alef				+ya2 before					-shadde instead of alef	souriya--> souriy
consonant or waw								-ya2					lubnan -> lubnaniy		or		hunululo  -->	hunululoy (???)
ta2	marbouta		+consonant before			-ya2 instead of ta2		makka --> makkiy
ya2												-add a shadde on ya2	jibouti --> jiboutiy

Notes:
-remove ta3reef if it is there, except for almanya, albanya, ...
-asma2 el mourakaba according to last word
  */

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
	long category_Nall_L_id=getID("category","Nall_L",QString("abstract=0 AND type=%1").arg((int)STEM));
	long category_Nall_id=getID("category","Nall",QString("abstract=0 AND type=%1").arg((int)STEM));
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
			//insert word as is
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
			//insert possessive form of the word
			QString possessive=get_Possessive_form(line);
			out << QString("Possesive form for '%1' is '%2'\n").arg(line).arg(possessive);
			stem_id=insert_item(STEM,removeDiactrics(possessive),possessive,(possessive.startsWith(lam)?"Nall_L":"Nall"),source_id,abstract_category_id,"Proper noun","","","");
			if (stem_id<0)
			{
				out<<"Error while adding Possessive at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
			if (addAbstractCategory("stem_category",abstract_Noun_Prop_id,-1,QString("stem_id=%1 AND category_id=%2").arg(stem_id).arg((possessive.startsWith(lam)?category_Nall_L_id:category_Nall_id)),false)==NULL)
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
	/*if (insert_buckwalter()<0)//generate_all_prefixes()
		return -1;*/
	if (insert_propernames()<0)
		return -1;
	/*QString word;
	in >>word;
	out << get_Possessive_form(word);*/
	return 0;
}

#endif // INSERT_SCRIPTS_H
