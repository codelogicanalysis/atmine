#ifndef AFFIXTREETRAVERSAL_H
#define AFFIXTREETRAVERSAL_H

#include "test.h"
#include "common.h"
#include "tree.h"
#include "database_info_block.h"
#include "inflections.h"
#include <QString>

class AffixTraversal {
protected:
	item_types type:2;
	bool appendEnabled:1;
	bool acceptOnly:1;
	ItemCatRaw2AbsDescPosMapPtr map;
private:
	inline QString appendToField(QString oldF, QString newF) {
		return (appendEnabled?oldF+newF:newF);
	}
	virtual void visit(node * n,QString affix,QString raw_data,long category_id, QString description, QString POS)=0;
	void traverseHelperLetter(node * n,QString affix,QString raw, QString desc, QString pos) {
		QVector<letter_node *> letters=n->getLetterChildren();
		for (int i=0;i<letters.size();i++) {
			if (letters[i]!=NULL) {
				QChar letter=letters[i]->getLetter();
				QString addedLetter="";
				if (letter!='\0')
					addedLetter+=letter;
				QString finalAffix=affix+addedLetter;
				traverseHelper(letters[i],finalAffix,raw,desc,pos);
			}
		}
	}
	void traverseHelper(node * n,QString affix,QString raw_data, QString description, QString POS) {
		if (!n->isLetterNode()) {
			result_node * r=(result_node* )n;
			QString inflectionRule=r->getInflectionRule();
			for (int i=0; i<r->raw_datas.size();i++) {
				QString raw=appendToField(raw_data,r->raw_datas[i].getActual());
				long cat_id=r->get_resulting_category_id();
				ItemEntryKey entry(r->get_affix_id(),r->get_previous_category_id(),r->raw_datas[i].getOriginal());
				ItemCatRaw2AbsDescPosMapItr itr=map->find(entry);
				while (itr!=map->end() && itr.key()==entry) {
					dbitvec d=itr.value().first;
					QString pos2=itr.value().third;
					QString pos=POS;
					applyPOSInflections(inflectionRule,pos2);
					applyPOSInflections(inflectionRule,pos,true);
					pos=appendToField(pos,pos2);
					QString added_desc=(*database_info.descriptions)[itr.value().second];
					applyDescriptionInflections(inflectionRule,added_desc);
					applyDescriptionInflections(inflectionRule,description,true);
					QString desc;
					if (type==SUFFIX){
						QString suffix_delimitor=" ";
						bool r=isReverseDirection(d);
						//qDebug()<<raw_data<<" "<<r;
						if (description[0]=='[' && description.size()>0 && description[description.size()-1]==']' && !added_desc.isEmpty())
							description="";
						if (description.contains("%1"))
							desc=description.arg(added_desc+(added_desc=="" || description=="%1"?"":suffix_delimitor));
						else {
							if (r)
								desc="%1"+added_desc+(added_desc=="" || description==""?"":suffix_delimitor)+description;
							else
								desc= description+(added_desc=="" || description==""?"":suffix_delimitor)+added_desc;
						}
						//desc=(isReverseDirection(d)?added_desc+(added_desc==""?"":" ")+description: description+(description==""?"":" ")+added_desc);
						//desc=(isReverseDirection(d)?added_desc+(added_desc==""?"":" ")+description: description+(description==""?"":" ")+added_desc);
					}else
						desc=appendToField(description,(description=="" || added_desc=="" || !appendEnabled?"":" + ")+added_desc);
					if (!acceptOnly || r->is_accept_state()) {
						QString temp=desc;
						temp.remove("%1");
						visit(n->parent,affix,raw,cat_id,temp,pos);
					}
					QString a=affix;
					if (!appendEnabled)
						a="";
					traverseHelperLetter(n,a,raw,desc,pos);
					itr++;
				}
			}
		} else {
			QList<result_node *> * results=n->getResultChildren();
			for (int i=0;i<results->size();i++) {
				traverseHelper((*results)[i],affix,raw_data,description,POS);
			}
			traverseHelperLetter(n,affix,raw_data,description,POS);
		}
	}
public:
	AffixTraversal(item_types type,bool append=false, bool acceptOnly=true) {
		this->type=type;
		appendEnabled=append;
		this->acceptOnly=acceptOnly;
		map=(type==PREFIX?database_info.map_prefix:database_info.map_suffix);
	}
	void operator ()() {
		node * top=(type==PREFIX? database_info.Prefix_Tree:database_info.Suffix_Tree)->getFirstNode();
		traverseHelper(top,"","","","");
	}
};

#endif // AFFIXTREETRAVERSAL_H
