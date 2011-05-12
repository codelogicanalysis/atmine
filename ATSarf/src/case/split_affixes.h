#ifndef SPLIT_AFFIXES_H
#define SPLIT_AFFIXES_H

#include <QDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QTextBrowser>
#include "Retrieve_Template.h"
#include "Search_by_item.h"
#include "Search_Compatibility.h"
#include "database_info_block.h"
#include "sql_queries.h"
#include "logger.h"
#include <iostream>

using namespace std;

class SplitDialog:public QDialog{
	Q_OBJECT
public:
	SplitDialog():QDialog() {
		errors=new QTextBrowser(this);
		errors->resize(errors->width(),50);
		errors_text=new QString();
		split=new QPushButton("&Split",this);
		exportLists=new QPushButton("&Export",this);
		reverse=new QPushButton("&Reverse Check",this);
		affixType=new QComboBox(this);
		affixType->addItem("Prefix",PREFIX);
		affixType->addItem("Suffix",SUFFIX);
		originalAffixList=new QTableWidget(0,7,this);
		compatRulesList=new QTableWidget(0,3,this);
		grid=new QGridLayout(this);
		grid->addWidget(split,0,0);
		grid->addWidget(affixType,0,1);
		grid->addWidget(exportLists,0,2);
		grid->addWidget(reverse,0,3);
		grid->addWidget(originalAffixList,1,0,1,4);
		grid->addWidget(compatRulesList,2,0,1,4);
		grid->addWidget(errors,3,0,1,4);
		connect(split,SIGNAL(clicked()),this,SLOT(split_clicked()));
		connect(exportLists,SIGNAL(clicked()),this,SLOT(export_clicked()));
		connect(affixType,SIGNAL(currentIndexChanged(int)),this,SLOT(affixType_currentIndexChanged(int)));
		connect(reverse,SIGNAL(clicked()),this,SLOT(reverse_clicked()));
		displayed_error.setString(errors_text);
		loadAffixList();
		loadCompatibilityList();
		this->resize(700,700);
		errors->setText(*errors_text);
		source_id=-1;
	}
	void loadAffixList() {
		QString table=interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
		Columns list;
		originalAffixList->clear();
		QStringList v;
		v<<table+" ID"<<"Affix"<<"Category"<<"Raw Data"<<"POS"<<"Description"<<"Reverse";
		originalAffixList->verticalHeader()->setHidden(true);
		originalAffixList->setHorizontalHeaderLabels(v);
		originalAffixList->setEditTriggers(QAbstractItemView::NoEditTriggers);
		originalAffixList->setSelectionBehavior(QAbstractItemView::SelectRows);
		originalAffixList->setSelectionMode(QAbstractItemView::SingleSelection);
		list.append(table+"_id");
		list.append("category_id");
		list.append("raw_data");
		list.append("POS");
		list.append("description_id");
		int size=0;
		for (int i=0;i<2;i++) {
			Retrieve_Template s(table+"_category",list,tr("reverse_description=%1 ORDER BY ").arg(i)+table+"_id ASC");
			int row=size;
			size+=s.size();
			originalAffixList->setRowCount(size);
			while (s.retrieve())
			{
				long id=s.get(0).toLongLong();
				originalAffixList->setItem(row,0,new QTableWidgetItem(tr("%1").arg(id)));
				originalAffixList->setItem(row,1,new QTableWidgetItem(getAffix(id)));
				originalAffixList->setItem(row,2,new QTableWidgetItem(database_info.comp_rules->getCategoryName(s.get(1).toLongLong())));
				originalAffixList->setItem(row,3,new QTableWidgetItem(s.get(2).toString()));
				originalAffixList->setItem(row,4,new QTableWidgetItem(s.get(3).toString()));
				long description_id=s.get(4).toLongLong();
				QString desc;
				if (description_id>0 && description_id<database_info.descriptions->size())
					desc= database_info.descriptions->at(description_id);
				else
					desc= QString::null;
				originalAffixList->setItem(row,5,new QTableWidgetItem(desc));
				originalAffixList->setItem(row,6,new QTableWidgetItem(tr("%1").arg(i)));
				row++;
			}
		}
	}
	void loadCompatibilityList() {
		QStringList v;
		QString table=interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
		v<<table+" Category 1"<<table+" Category 2"<<"Resulting Category";
		compatRulesList->setHorizontalHeaderLabels(v);
		compatRulesList->setEditTriggers(QAbstractItemView::NoEditTriggers);
		compatRulesList->setSelectionBehavior(QAbstractItemView::SelectRows);
		compatRulesList->setSelectionMode(QAbstractItemView::NoSelection);
		compatRulesList->setRowCount(0);
	}

public slots:
	void reverse_clicked() {
		reverse_action();
	}
	void split_clicked() {
		split_action();
	}
	void export_clicked() {
		item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
		QFile f(tr("%1List.txt").arg(interpret_type(t)));
		assert(f.open(QIODevice::WriteOnly));
		QTextStream file(&f);
		file.setCodec("utf-8");
		for (int i=0;i<originalAffixList->rowCount();i++) {
			//long affix_id=originalAffixList->item(i,0)->text().toLongLong();
			QString affix=originalAffixList->item(i,1)->text();
			QString category=originalAffixList->item(i,2)->text();
			QString raw_data=originalAffixList->item(i,3)->text();
			QString pos=originalAffixList->item(i,4)->text();
			QString description=originalAffixList->item(i,5)->text();
			QString reverse_description=originalAffixList->item(i,6)->text();
			file<<affix<<"\t"<<raw_data<<"\t"<<category<<"\t"<<description<<"\t"<<pos<<"\t"<<reverse_description<<"\n";
		}
		f.close();
		rules rule=(t==PREFIX?AA:CC);
		QFile f2(tr("%1.txt").arg(interpret_type(rule)));
		assert(f2.open(QIODevice::WriteOnly));
		QTextStream file2(&f2);
		file2.setCodec("utf-8");
		Retrieve_Template r("compatibility_rules","category_id1","category_id2","resulting_category",tr("type=%1").arg(rule));
		while (r.retrieve()) {
			file2<<database_info.comp_rules->getCategoryName(r.get(0).toLongLong())<<"\t"
				 <<database_info.comp_rules->getCategoryName(r.get(1).toLongLong())<<"\t"
				 <<database_info.comp_rules->getCategoryName(r.get(2).toLongLong())<<"\n";
		}
		f2.close();
	}
	void affixType_currentIndexChanged(int) {
		loadAffixList();
		loadCompatibilityList();
		errors->clear();
	}

private:
	void split_action();
	void reverse_action();
	QString getAffix(long id) {
		item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
		QString table=interpret_type(t);
		Retrieve_Template s(table,"name",tr("id=%1").arg(id));
		if (s.retrieve())
			return s.get(0).toString();
		else
			return "--";
	}
	bool hasSubjectObjectInconsistency(QString descriptionDesired,QString descriptionFound) { //works for prefixes
		descriptionFound.replace("they","them");
		descriptionFound.replace("he/","him/");
		descriptionFound.replace("she","her");
		descriptionFound.replace("I","me");
		descriptionFound.replace("we","us");
		return (descriptionDesired.startsWith(descriptionFound)
				&& (descriptionDesired.endsWith("to") || descriptionDesired.endsWith("(to)")  ));
	}
	int getRow(const QString & affix,const QString & raw_data, const QString & pos, const QString & description) {
		if (pos.isEmpty())
			return -1;
		item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
		int rowCount=originalAffixList->rowCount();
		for (int i=0;i<rowCount;i++) {
			//long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
			QString affix1=originalAffixList->item(i,1)->text();
			QString category1=originalAffixList->item(i,2)->text();
			QString raw_data1=originalAffixList->item(i,3)->text();
			QString pos1=originalAffixList->item(i,4)->text();
			QString description1=originalAffixList->item(i,5)->text();
			if (	affix1==affix &&
					raw_data1==raw_data &&
					pos1==pos ) {
				if (description1==description)
					return i;
				else if (t==PREFIX && hasSubjectObjectInconsistency(description,description1)){ //prefix subject/object confusion for "for"
					QString newCat=category1+"(object)";
					long added_affix_id=insert_item(PREFIX,affix,raw_data,newCat,source_id,NULL,description,pos);
					//TODO: solve staleness
					/*system(QString(tr("rm ")+compatibility_rules_path+" "+description_path).toStdString().data());
					database_info.comp_rules->buildFromFile();
					database_info.fill((ATMProgressIFC*)parentWidget());*/
					warning<< "Adding prefix ("<<affix<<","<<raw_data<<","<<newCat<<","<<description<<","<<pos<<"\n";
					assert(added_affix_id>=0);
					originalAffixList->setRowCount(rowCount+1);
					originalAffixList->setItem(rowCount,0,new QTableWidgetItem(tr("%1").arg(added_affix_id)));
					originalAffixList->setItem(rowCount,1,new QTableWidgetItem(affix));
					originalAffixList->setItem(rowCount,2,new QTableWidgetItem(newCat));
					originalAffixList->setItem(rowCount,3,new QTableWidgetItem(raw_data));
					originalAffixList->setItem(rowCount,4,new QTableWidgetItem(pos));
					originalAffixList->setItem(rowCount,5,new QTableWidgetItem(description));
					return rowCount;
				}
			}

		}
		return -1;
	}

public:
	int source_id;
	QTextEdit * affix;
	QPushButton * split, *exportLists, *reverse;
	QComboBox * affixType;
	QTextBrowser * errors;
	QString * errors_text;
	QTableWidget * originalAffixList, * compatRulesList;
	QGridLayout * grid;

	~SplitDialog() {
		delete affix;
		delete split;
		delete exportLists;
		delete reverse;
		delete affixType;
		delete errors;
		delete errors_text;
		delete originalAffixList;
		delete compatRulesList;
		delete grid;
	}
};


#endif // SPLIT_AFFIXES_H
