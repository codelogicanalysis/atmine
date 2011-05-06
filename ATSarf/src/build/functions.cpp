#include <QFile>
#include <QStringList>
#include <QDir>
#include <QString>
#include "functions.h"
#include "sql_queries.h"
#include "text_handling.h"
#include "diacritics.h"
#include "Search_Compatibility.h"

bool pos=false;
long abstract_possessive;

int insert_rules_for_Nprop_Al()//copies all rules of Nprop to Aprop_Al and adds to them the support for Al
{
	int source_id=insert_source("Jad and Hamza modifications","as needed","Hamza Harkous and Jad Makhlouta");
	QStringList added_prefixes;
	added_prefixes<<"NPref-Al"<<"NPref-BiAl"<<"NPref-Lil"<<"NPref-LiAl";
	long prefix_cat_id,suffix_cat_id;
	long stem_cat_id_Nprop=getID("category","Nprop");
	long stem_cat_id_Nprop_Al=insert_category("Nprop_Al",STEM,source_id,false);
	Search_Compatibility search_suffixes(BC,stem_cat_id_Nprop,true);
	while (search_suffixes.retrieve(suffix_cat_id))
	{
		Search_Compatibility search_prefixes(AB,stem_cat_id_Nprop,false);
		//qDebug()<<search_prefixes.size()<<" "<<search_suffixes.size()<<"\n";
		while (search_prefixes.retrieve(prefix_cat_id))
		{
			insert_compatibility_rules(AB,prefix_cat_id,stem_cat_id_Nprop_Al,source_id);
			insert_compatibility_rules(AC,prefix_cat_id,suffix_cat_id,source_id);
			insert_compatibility_rules(BC,stem_cat_id_Nprop_Al,suffix_cat_id,source_id);
		}
		QString prefix_cat;
		foreach(prefix_cat,added_prefixes)
		{
			prefix_cat_id=getID("category",prefix_cat);
			insert_compatibility_rules(AB,prefix_cat_id,stem_cat_id_Nprop_Al,source_id);
			insert_compatibility_rules(AC,prefix_cat_id,suffix_cat_id,source_id);
			insert_compatibility_rules(BC,stem_cat_id_Nprop_Al,suffix_cat_id,source_id);
		}
	}
	return 0;
}

//utility function for insert scripts used in insert_propernames() and insert_placenames()
int insert_NProp(QString word,QList<long> * abstract_categories, int source_id, QString description="Proper noun")
{
	bool hasAL= removeAL(word);
	//insert word as is
	int stem_id=insert_item(STEM,removeDiacritics(word),word,(hasAL?"Nprop_Al":"Nprop_Al"),source_id,abstract_categories,description,"","","");
	if (stem_id<0)
	{
		error <<"while adding stem: "<<word<<"'\n";
		return -1;
	}
	//insert possessive form of the word
	if (!pos)
	{
		abstract_possessive=insert_category("POSSESSIVE",STEM,dbitvec(max_sources),true);
		pos=true;
	}
	abstract_categories->append(abstract_possessive);
	QString possessive=get_Possessive_form(word);
	//out << QString("Possesive form for '%1' is '%2'\n").arg(word).arg(possessive);
		stem_id=insert_item(STEM,removeDiacritics(possessive),possessive,(possessive.startsWith(lam)?"Nall_L":"Nall"),source_id,abstract_categories,QString("possessive form of ").append(description),"","","");
	if (stem_id<0)
	{
		error<<"while adding Possessive: "<<word<<"'\n";
		return -1;
	}
	return 0;
}

//main functions
int insert_buckwalter()
{
	int source_id=insert_source("Buckwalter Dictionaries","modifying aramorph.pl + insert_buckwalter() c++ code fragment","Jad Makhlouta");
	//items
	const QString item_files[3]= {	"../../src/buckwalter scripts/list_of_prefixes.txt",
									"../../src/buckwalter scripts/list_of_suffixes.txt",
									"../../src/buckwalter scripts/list_of_stems.txt"};
	const item_types types[3] ={ PREFIX, SUFFIX,STEM};
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
				item=item.replace("{",QString(alef_wasla));
				item=item.replace("`",QString(aleft_superscript));
				item=item.replace("V",QString(veh));
				//out << "Replaced "<<interpret_type(types[j])<<": "<<before<<" by "<<item<<"\n";
			}
			if (raw_data.contains("{") || raw_data.contains("`") || raw_data.contains("V"))
			{
				QString before=raw_data;
				raw_data=raw_data.replace("{",QString(alef_wasla));
				raw_data=raw_data.replace("`",QString(aleft_superscript));
				raw_data=raw_data.replace("V",QString(veh));
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
						//out<< QString("Inserted new Abstract Category '%1'\n").arg(abstract_category);
					}
				}
			}
			QList<long> *abstract_categories=new QList<long>();
			abstract_categories->append(abstract_id);
			if ((types[j]==STEM?insert_item(STEM,item,raw_data,category,source_id,abstract_categories,description,POS,"",lemmaID)<0:insert_item(types[j],item,raw_data,category,source_id,NULL,description,POS,"","")<0))
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(interpret_type(types[j]));
		input.close();
	}
	//compatibility rules
	const QString rules_files[5]= {"../../src/buckwalter scripts/tableAB", \
								  "../../src/buckwalter scripts/tableBC", \
								  "../../src/buckwalter scripts/tableAC", \
								  "../../src/buckwalter scripts/tableAA", \
								  "../../src/buckwalter scripts/tableCC"};
	const rules rule[5] ={ AB, BC, AC, AA, CC};
	for (int j=0;j<5;j++)
	{
		QFile input(rules_files[j]);
		if (!input.open(QIODevice::ReadWrite))
		{
			out << QString("File %1 not found\n").arg(rules_files[j]);
			if (j>3)
				continue;
			else
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
			if (line.isEmpty() || line.startsWith(";")) //ignore empty lines and comments if they exist
				continue;
			QStringList entries=line.split(" ",QString::KeepEmptyParts);
			if (entries.size()<2)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
			QString cat1=entries[0];
			QString cat2=entries[1];
			QString resCat=cat2;
			if (rule[j]==AA || rule[j]==CC)	{
				if (entries.size()<3) {
					out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
					return -1;
				} else {
					resCat=entries[2];
				}

			}
			if (insert_compatibility_rules(rule[j],cat1,cat2,resCat,source_id)<0)
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
	QDir folder("../../dic/N","*.txt");
	if (!folder.exists())
	{
		out << "Invalid Folder\n";
		return -1;
	}
	long abstract_Noun_Prop_id=insert_category("NOUN_PROP",STEM,dbitvec(max_sources),true); //returns id if already present
	long abstract_people_names=insert_category("Name of Person",STEM,dbitvec(max_sources),true); //returns id if already present
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
				QString ab=file_name.split(".").at(0);
				if (ab!="Compound Names")
					abstract_category_id=insert_category(ab,STEM,source_id,true);
				else
					abstract_category_id=insert_category("Male Names",STEM,source_id,true);
				continue;
			}
			if (line.isNull())
			{
				line_num--; //finished
				break;
			}
			if (line.isEmpty()) //ignore empty lines if they exist
				continue;
			QList<long> * abstract_categories=new QList<long>();
			abstract_categories->append(abstract_Noun_Prop_id);
			abstract_categories->append(abstract_category_id);
			abstract_categories->append(abstract_people_names);
			if (insert_NProp(line, abstract_categories,source_id,"Name of Person")<0)
			{
				out<<"Error at line "<<line_num<<": '"<<line<<"'\n";
				return -1;
			}
		}
		out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name);
		input.close();
	}
	return 0;
}

int insert_placenames() //not yet complete
{
		QDir folder("../../dic/P");
	if (!folder.exists())
	{
		out << "Invalid Folder\n";
		return -1;
	}
	int folders_source_id=insert_source("ar.wikipedia.org/","direct copy from html and dividing them into folders and subfiles according to continents and cities/towns","Hamza Harkous");

	long abstract_Noun_Prop_id=insert_category("NOUN_PROP",STEM,dbitvec(max_sources),true);////returns id if already present
	long abstract_place_names=insert_category("Name of Place",STEM,dbitvec(max_sources),true);////returns id if already present
	long abstract_continent_name=insert_category("Continent",STEM,dbitvec(max_sources),true);////returns id if already present
	long abstract_country_name=insert_category("Country",STEM,dbitvec(max_sources),true);////returns id if already present
	long abstract_city_name=insert_category("City/Town",STEM,dbitvec(max_sources),true);////returns id if already present
	QString file_name;
	QStringList h=folder.entryList();
	//qDebug()<<"size: "<<h.size();
	foreach (file_name,folder.entryList())
	{
		if (file_name.endsWith(".txt"))
		{
			if (file_name.startsWith("Google"))
			{
				out << QString("Ignored %1 file\n").arg(file_name);
				continue;
			}
			long abstract_category_id;
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
				QList<long> * abstract_categories=new QList<long>();;
				abstract_categories->append(abstract_Noun_Prop_id);
				abstract_categories->append(abstract_category_id);
				abstract_categories->append(abstract_place_names);
				if (insert_NProp(line, abstract_categories,source_id,file_name.split(".").at(0))<0)
				{/*return -1*/;}
			}
			out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name);
			input.close();
		}
		else if (!file_name.contains('.'))//assuming this means folder
		{
			QDir folder2(folder.absolutePath().append(QString("/").append(file_name)),"*.txt");
			if (!folder2.exists())
			{
				out << "Unexpected Error: Folder that was assumed to exist does not\n";
				return -1;
			}
			QStringList continent=file_name.split("-");
			QString continent_english=continent[0];
			if (continent.count()>1)
			{
				QString continent_arabic=continent[1];
				QList<long> * abstract_categories=new QList<long>();
				abstract_categories->clear();
				abstract_categories->append(abstract_Noun_Prop_id);
				abstract_categories->append(abstract_continent_name);
				abstract_categories->append(abstract_place_names);
				if (insert_NProp(continent_arabic, abstract_categories,folders_source_id,continent_english)<0)
				{/*return -1*/;}
			}
			dbitvec sources(max_sources);
			sources.reset();
			long continent_id=insert_category(continent_english,STEM,folders_source_id,true);
			QString file_name2;
			foreach (file_name2,folder2.entryList())
			{
				QFile input(folder2.absolutePath().append(QString("/").append(file_name2)));
				if (!input.open(QIODevice::ReadWrite))
				{
					out << "Unexpected Error: File not found\n";
					return 1;
				}
				QTextStream file(&input);
				file.setCodec("utf-8");
				int line_num=0;
				QStringList country=file_name2.split("-");
				QString country_english=country[0];
				if (country.count()>1)
				{
					QString country_arabic=country[1].split(".")[0];
					QList<long> * abstract_categories=new QList<long>();
					abstract_categories->append(abstract_Noun_Prop_id);
					abstract_categories->append(abstract_country_name);
					abstract_categories->append(abstract_place_names);
					abstract_categories->append(continent_id);
					if (insert_NProp(country_arabic, abstract_categories,folders_source_id,country_english)<0)
					{	/*return -1*/;}
				}
				long country_id=insert_category(country_english,STEM,folders_source_id,true);
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
					if (line.split(" ")[0].length()==1) //ignore lines that are just for sorting in alphabatical order
						continue;
					QList<long> * abstract_categories=new QList<long>();
					abstract_categories->append(abstract_Noun_Prop_id);
					abstract_categories->append(abstract_city_name);
					abstract_categories->append(abstract_place_names);
					abstract_categories->append(continent_id);
					abstract_categories->append(country_id);
					QString alltext=line.split('(')[0];
					QStringList city=alltext.split(QRegExp("[.-,]"));
					if (city.count()>1) //add all the text in addition to the primary part
						if (insert_NProp(alltext, abstract_categories,folders_source_id,"city/town in "+country_english)<0)
						{/*return -1*/;}
					if (insert_NProp(city[0], abstract_categories,folders_source_id,"city/town in "+country_english)<0)
					{/*return -1*/;}
				}
				out <<QString("\nSuccessfully processed all %1 %2 entries\n").arg(line_num).arg(file_name2);
				input.close();
			}
		}
	}
	return 0;
}
