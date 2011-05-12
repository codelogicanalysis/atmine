#include <QFile>
#include <QDataStream>
#include "tree.h"
#include "sql_queries.h"
#include "Search_Compatibility.h"
#include "Search_by_category.h"
#include "Search_by_item.h"
#include "text_handling.h"
#include <assert.h>
#include "database_info_block.h"
#include "Retrieve_Template.h"


#ifdef LOAD_FROM_FILE
#ifdef REDUCE_THRU_DIACRITICS
inline QString cache_version()
{
	return "RD";
}
#elif defined(MEMORY_EXHAUSTIVE)
inline QString cache_version()
{
	return "ME";
}
#else
inline QString cache_version()
{
	return "ND";
}
#endif
#endif

inline bool isAcceptState(item_types type,long cat_r_id) {
	bool isAccept=false;
	if (type==PREFIX) {
		Retrieve_Template existABcheck("compatibility_rules","COUNT(*)",QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)AB));
		if (existABcheck.retrieve() && existABcheck.get(0).toInt()>0) {
			Retrieve_Template existACcheck("compatibility_rules","COUNT(*)",QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)AC));
			if (existACcheck.retrieve() && existACcheck.get(0).toInt()>0) {
				isAccept=true;
			}
		}
	} else {
		Retrieve_Template existACcheck("compatibility_rules","COUNT(*)",QString("category_id2=%1 AND type=%2").arg(cat_r_id).arg((int)AC));
		if (existACcheck.retrieve() && existACcheck.get(0).toInt()>0) {
			Retrieve_Template existBCcheck("compatibility_rules","COUNT(*)",QString("category_id2=%1 AND type=%2").arg(cat_r_id).arg((int)BC));
			if (existBCcheck.retrieve() && existBCcheck.get(0).toInt()>0) {
				isAccept=true;
			}
		}
	}
	return isAccept;
}

inline bool hasCompatibleAffixes(item_types type,long cat_r_id) {
	bool hasComp=false;
	Retrieve_Template check("compatibility_rules","COUNT(*)",QString("category_id1=%1 AND type=%2").arg(cat_r_id).arg((int)(type==PREFIX?AA:CC)));
	if (check.retrieve() && check.get(0).toInt()>0) {
		hasComp=true;
	}
	return hasComp;
}

void tree::print_tree_helper(node * current_node, int level)
{
    out<<QString().fill(' ',level*7)<<current_node->to_string(isAffix)<<"\n";
	QVector<letter_node* > list=current_node->getLetterChildren();
	for(int i=0;i<list.count();i++)
		print_tree_helper(list.at(i),level+1);
	QList<result_node*>* list2=current_node->getResultChildren();
	for(int i=0;i<list2->count();i++)
		print_tree_helper(list2->at(i),level+1);
}
int tree::build_helper(item_types type, long cat_id1, int size, node * current)
{
    if (size<=0)
            return 0;
    long cat_id2,cat_r_id;
    Search_Compatibility s2((type==PREFIX?AA:CC),cat_id1);
    while (s2.retrieve(cat_id2,cat_r_id))
    {
		bool isAccept=isAcceptState(type,cat_r_id);
		if (isAccept || hasCompatibleAffixes(type,cat_r_id)) { //dont add to tree branches that have no rules and connect to nothing else that may have rules
			Search_by_category s3(cat_id2);
		#ifdef MEMORY_EXHAUSTIVE
			all_item_info inf;
			while(s3.retrieve(inf))
			{
					QString name= getColumn(interpret_type(type),"name",inf.item_id);
					node * next=addElement(name,inf.item_id,cat_id2,cat_r_id,isAccept,inf.raw_data,inf.description,current);
		#elif defined(REDUCE_THRU_DIACRITICS)
			all_item_info inf;
			while(s3.retrieve(inf))
			{
					QString name= getColumn(interpret_type(type),"name",inf.item_id);
					node * next=addElement(name,inf.item_id,cat_id2,cat_r_id,isAccept,inf.raw_data,current);
		#else
			long long affix_id;
			while(s3.retrieve(affix_id))
			{
					QString name= getColumn(interpret_type(type),"name",affix_id);
					node * next=addElement(name,affix_id,cat_id2,cat_r_id,isAccept,current);

		#endif
					build_helper(type,cat_r_id,size-name.length(),next);
			}
		}
    }
    return 0;
}
#ifdef MEMORY_EXHAUSTIVE
node* tree::addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,QString raw_data,QString description,node * current)
#elif defined (REDUCE_THRU_DIACRITICS)
node* tree::addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,QString raw_data,node * current)
#else
node* tree::addElement(QString letters, long affix_id,long category_id, long resulting_category_id,bool isAccept,node * current)
#endif
{
	assert (equal(letters,raw_data));
#ifdef LOAD_FROM_FILE
	if (file!=NULL)
	{
		(*file)<<letters<<affix_id<<category_id<<resulting_category_id<<isAccept
			#if defined (REDUCE_THRU_DIACRITICS)
				<<raw_data
			#elif defined (MEMORY_EXHAUSTIVE)
				<<raw_data<<description
			#endif
				<<generateNodeID(current);
	}
#endif
	//pre-condition: assumes category_id is added to the right place and results in the appropraite resulting_category
	if (current->isLetterNode() && current!=base) {
		error << "Unexpected Error: provided node was a letter node and not a result one\n";
	#ifdef LOAD_FROM_FILE
		if (file!=NULL)
			(*file)<<generateNodeID(NULL);
	#endif
		return NULL;
    }
    QChar current_letter;
	//QList<letter_node *>* current_letter_children;
	letter_node* matching_letter_node=NULL;
	int i;
    if (letters.count()==0)
    {
		current_letter='\0';
		if (current==base)
			goto result;
    }
    else
		current_letter=letters[0];
    i=0;
    do
    {
		matching_letter_node=current->getLetterChild(current_letter);
		if (matching_letter_node!=NULL)
		{
				current=matching_letter_node;
				i++;
				current_letter=letters[i];
		}
		else
			break;
    }while(i<letters.count());
    if (letters.count()==0 && i==0)
    {
		//add null letter
		letter_node* new_node=new letter_node('\0');
		current->addChild(new_node);
		current=new_node;
		letter_nodes++;
    }
    for (;i<letters.count();i++)
    {
		//add necessary letters
		letter_node* new_node=new letter_node(letters[i]);
		current->addChild(new_node);
		current=new_node;
		letter_nodes++;
    }
result:
	int size=current->getResultChildren()->size();
	for (int i=0;i<size;i++) //check if this result node is already present
    {
		result_node * old_result=current->getResultChildren()->at(i);
		if (old_result->get_previous_category_id()==category_id && old_result->get_resulting_category_id()==resulting_category_id && old_result->get_affix_id()==affix_id)
		{
		#ifdef MEMORY_EXHAUSTIVE
			((result_node*)old_result)->addPair(raw_data,description);
		#elif defined(REDUCE_THRU_DIACRITICS)
			old_result->add_raw_data(raw_data);
		#endif
		#ifdef LOAD_FROM_FILE
			if (file!=NULL)
				(*file)<<generateNodeID(old_result);
		#endif
			return old_result;
		}
    }
#ifdef MEMORY_EXHAUSTIVE
	result_node * result=new result_node(affix_id,category_id,resulting_category_id,isAccept,raw_data,description);
#elif defined(REDUCE_THRU_DIACRITICS)
	result_node * result=new result_node(affix_id,category_id,resulting_category_id,isAccept,raw_data);
#else
	result_node * result=new result_node(affix_id,category_id,resulting_category_id,isAccept);
#endif
    current->addChild(result);
    current=result;
    result_nodes++;
#ifdef LOAD_FROM_FILE
	if (file!=NULL)
		(*file)<<generateNodeID(current);
#endif
    return current;
    //post-condition: returns node of resulting category reached after addition
}

tree::tree()
{
    base= new letter_node('\0');
    letter_nodes=1;
    result_nodes=0;
    isAffix=false;
#ifdef LOAD_FROM_FILE
	file=NULL;
#endif
}
tree::tree(item_types type)
{
    base= new letter_node('\0');
    letter_nodes=1;
    result_nodes=0;
    isAffix=true;
#ifdef LOAD_FROM_FILE
	file=NULL;
#endif
    build_affix_tree(type);
}
bool tree::getAffixType(item_types &type)
{
	if (isAffix)
	{
		type=this->type;
		return true;
	}
	else
		return false;
}
#if !defined(MEMORY_EXHAUSTIVE) && !defined(REDUCE_THRU_DIACRITICS)
void tree::sample()
{
    reset();
    letter_node * A0=new letter_node('A');
    letter_node * B0=new letter_node('B');
    letter_node * C0=new letter_node('C');
    letter_node * D0=new letter_node('D');
    letter_node * B1=new letter_node('B');
    letter_node * C1=new letter_node('C');
    letter_node * D1=new letter_node('D');
    letter_node * C3=new letter_node('C');
    letter_node * D3=new letter_node('D');
    letter_node * F1=new letter_node('F');
    letter_node * E1=new letter_node('E');
    letter_node * G1=new letter_node('G');
    letter_node * G2=new letter_node('G');
    letter_node * F2=new letter_node('F');
    result_node * rc1= new result_node(-1,0,1);
    result_node * rc2= new result_node(-1,2,2);
    result_node * rc3= new result_node(-1,0,3);
    result_node * rc5= new result_node(-1,3,5);
    result_node * rc6= new result_node(-1,4,6);
    result_node * rc7= new result_node(-1,9,7);
    result_node * rc8= new result_node(-1,1,8);
    result_node * rc9= new result_node(-1,1,9);
    result_node * rc9_= new result_node(-1,3,9);
    base->addChild(A0);
    A0->addChild(rc1);
    A0->addChild(B0);
    rc1->addChild(B1);
    B1->addChild(rc2);
    rc2->addChild(C0);
    C0->addChild(D0);
    D0->addChild(rc3);
    B0->addChild(rc5);
    rc5->addChild(C1);
    C1->addChild(D1);
    D1->addChild(rc6);
    B0->addChild(C3);
    C3->addChild(rc7);
    rc7->addChild(D3);
    rc7->addChild(F1);
    D3->addChild(E1);
    E1->addChild(rc8);
    F1->addChild(G1);
    G1->addChild(rc9);
    C3->addChild(F2);
    F2->addChild(G2);
    G2->addChild(rc9_);
    letter_nodes=15;
    result_nodes=9;
    isAffix=false;
}
#endif
#ifdef LOAD_FROM_FILE
int tree::generateNodeID(node *n)
{
	int curr_id;
	IDMap::iterator it=idMap.find(n);
	if (it==idMap.end())
	{
		if (n!=base)
			curr_id=++last_id;
		else
			curr_id=0; //base is number 0
		idMap.insert(n,curr_id);
	}
	else
		curr_id=it.value();
	return curr_id;
}
node* tree::getNodeID(int num)
{
	ReverseIDMap::iterator it=reverseIDMap.find(num);
	node * n;
	if (num==0)//base is number 0
		n=base;
	else
	{
		assert (it!=reverseIDMap.end());
		n=it.value();
	}
	return n;
}
void tree::setNodeID(int num, node * n)
{
	ReverseIDMap::iterator it=reverseIDMap.find(num);
	if(it==reverseIDMap.end())
		reverseIDMap.insert(num,n);
	else
		assert(n==it.value());
}
#endif
int tree::build_affix_tree_from_file(item_types type)
{
	//out<<QDateTime::currentDateTime().time().toString()<<"\n";
#ifndef LOAD_FROM_FILE
	return build_affix_tree(type);
#else
	file=NULL;
	reset();
	isAffix=true;
	this->type=type;
	QString fileName;
	if (type==PREFIX)
		fileName=prefix_tree_path;
	else if (type==SUFFIX)
		fileName=suffix_tree_path;
	reverseIDMap.clear();
	QFile file(fileName.toStdString().data());
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);    // read the data serialized from the file
		QString version;
		in >> version;
		if (version==cache_version())
		{
			int num1,num2;
			QString letters; long affix_id;long category_id; long resulting_category_id;bool isAccept;
		#if defined (REDUCE_THRU_DIACRITICS)
			QString raw_data;
		#elif defined (MEMORY_EXHAUSTIVE)
			QString raw_data;QString description;
		#endif
			while(!in.atEnd())
			{
				in>>letters>>affix_id>>category_id>>resulting_category_id>>isAccept
					#if defined (REDUCE_THRU_DIACRITICS)
							>>raw_data
					#elif defined (MEMORY_EXHAUSTIVE)
							>>raw_data>>description
					#endif
							>>num1>>num2;
				node * n=addElement(letters,affix_id,category_id,resulting_category_id,isAccept,
					#if defined (REDUCE_THRU_DIACRITICS)
							raw_data,
					#elif defined (MEMORY_EXHAUSTIVE)
							raw_data,description,
					#endif
							getNodeID(num1));
				setNodeID(num2,n);
			}
			file.close();
			return 0;
		}
		else
		{
			file.close();
			return build_affix_tree(type);
		}
	}
	else
		return build_affix_tree(type);
#endif
}
int tree::build_affix_tree(item_types type)
{
    reset();
    isAffix=true;
    this->type=type;
	database_info.prgsIFC->setCurrentAction(interpret_type(type).toUpper()+" TREE");
#ifdef LOAD_FROM_FILE
	QString fileName;
	if (type==PREFIX)
		fileName=prefix_tree_path;
	else if (type==SUFFIX)
		fileName=suffix_tree_path;
	last_id=1;//bc base =0
	idMap.clear();

	QFile rawFile(fileName.toStdString().data());
	if (rawFile.open(QIODevice::WriteOnly))
	{
		file=new QDataStream(&rawFile);   // we will serialize the data into the file
		(*file)<<cache_version();
	}
	else
	{
		error <<"Unexpected Error: Unable to write AFFIX Table to file\n";
		file=NULL;
	}
#endif
	QSqlQuery query(db);
    QString stmt=QString("SELECT id, name FROM %1").arg(interpret_type(type));
    QString name;
    unsigned long affix_id1;
    if (!execute_query(stmt,query))
			return -1;
	int size=query.size();
	int i=0;
    while (query.next())
    {
		name=query.value(1).toString();
		affix_id1=query.value(0).toULongLong();
		Search_by_item s1(type,affix_id1);
#ifdef MEMORY_EXHAUSTIVE
		minimal_item_info inf;
		while(s1.retrieve(inf))
		{
			bool isAccept=isAcceptState(type,inf.category_id);
			if (isAccept || hasCompatibleAffixes(type,inf.category_id)) {
				node * next=addElement(name,affix_id1,inf.category_id,inf.category_id,isAccept,inf.raw_data,inf.description,base);
				if (type!=STEM)
					build_helper(type,inf.category_id,6-name.length(),next);
			}
		}
#elif defined(REDUCE_THRU_DIACRITICS)
		minimal_item_info inf;
		while(s1.retrieve(inf))
		{
			bool isAccept=isAcceptState(type,inf.category_id);
			if (isAccept || hasCompatibleAffixes(type,inf.category_id)) {
				node * next=addElement(name,affix_id1,inf.category_id,inf.category_id,isAccept,inf.raw_data,base);
				if (type!=STEM)
					build_helper(type,inf.category_id,6-name.length(),next);
			}
		}
#else
		long cat_id;
		while(s1.retrieve(cat_id))
		{
			bool isAccept=isAcceptState(type,cat_id);
			if (isAccept || hasCompatibleAffixes(type,cat_id)) {
				node * next=addElement(name,affix_id1,cat_id,cat_id,isAccept,base);
				if (type!=STEM)
					build_helper(type,cat_id,6-name.length(),next);
			}
		}

#endif
		i++;
		database_info.prgsIFC->report((double)i/size*100+0.5);
    }
#ifdef LOAD_FROM_FILE
	rawFile.close();
	if (file!=NULL)
		delete file;
	file=NULL;
#endif
    return 0;
}
void tree::print_tree()
{
    if (isAffix)
            out	<<QString().fill('-',40)<<"\n"
                    <<"\t"<<interpret_type(type)<<" Tree\n"
                    <<QString().fill('-',40)<<"\n";
    print_tree_helper(base,0);
    out	<<QString().fill('-',40)<<"\n"
            <<"letter nodes count= "<<letter_nodes<<"\n"
            <<"result nodes count= "<<result_nodes<<"\n"
            <<QString().fill('-',40)<<"\n";
}
