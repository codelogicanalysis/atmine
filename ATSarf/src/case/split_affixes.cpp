#include "split_affixes.h"
#include "test.h"
#include "Search_by_category.h"

void SplitDialog::findDuplicates() {
	int rowCount=originalAffixList->rowCount();
	for (int i=0;i<rowCount;i++) {
		//long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
		QString affix1=originalAffixList->item(i,1)->text();
		QString category1=originalAffixList->item(i,2)->text();
		QString raw_data1=originalAffixList->item(i,3)->text();
		QString pos1=originalAffixList->item(i,4)->text();
		QString description1=originalAffixList->item(i,5)->text();
		for (int j=i+1;j<rowCount;j++) {
			//long affix_id2=originalAffixList->item(j,0)->text().toLongLong();
			QString affix2=originalAffixList->item(j,1)->text();
			QString category2=originalAffixList->item(j,2)->text();
			QString raw_data2=originalAffixList->item(j,3)->text();
			QString pos2=originalAffixList->item(j,4)->text();
			QString description2=originalAffixList->item(j,5)->text();
			if (	affix2==affix1 &&
					raw_data2==raw_data1 &&
					pos2==pos1 &&
					description2==description1
				) {
				warning<<"("<<affix2<<","<<raw_data2<<","<<description2<<","<<pos2<<") found with 2 categories: "<<category1<<" & "<<category2<<"\n";
			}
		}
	}
	errors->setText(*errors_text);
}

void SplitDialog::makeNonAcceptStatesAccept() {
	item_types type=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(type==PREFIX?AA:CC);
	Retrieve_Template t("compatibility_rules","category_id2","resulting_category", tr("type=%1 AND category_id1=%2").arg(rule).arg(cat_empty));
	while (t.retrieve()) {
		long category_id=t.get(0).toLongLong();
		long resulting_category=t.get(1).toLongLong();
		if (!isAcceptState(type,resulting_category))
			continue;
		Search_by_category c(resulting_category);
		if (c.size()> 0)
			continue;
		Retrieve_Template t2("compatibility_rules","category_id2",
							tr("type=%1 AND category_id1=%2 AND resulting_category=%3 AND category_id2<>%4").arg(rule).arg(cat_empty).arg(resulting_category).arg(category_id));
		if (t2.size()>0)
			continue;
		QString cat=database_info.comp_rules->getCategoryName(category_id);
		QString catres=database_info.comp_rules->getCategoryName(resulting_category);
		warning << QString("Set category %1 back into %2 \n").arg(cat).arg(catres);
		QSqlQuery query(db);
		QString stmt=QString(tr("UPDATE %1_category ")+
								"SET category_id=%3 "+
								"WHERE category_id=%2")
					  .arg(interpret_type(type)).arg(category_id).arg(resulting_category);
		execute_query(stmt,query);
		assert(query.numRowsAffected()>0);

		stmt=QString(tr("DELETE  ")+
						"FROM  compatibility_rules "+
						"WHERE type=%1 AND category_id1=%2 AND category_id2=%3")
					  .arg((int)rule).arg(cat_empty).arg(category_id);
		execute_query(stmt,query);
		//assert(query.numRowsAffected()>0);

		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET category_id1=%3 "+
						"WHERE type=%1 AND category_id1=%2")
			  .arg((int)rule).arg(category_id).arg(resulting_category);
		execute_query(stmt,query);
		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET category_id2=%3 "+
						"WHERE type=%1 AND category_id2=%2")
			  .arg((int)rule).arg(category_id).arg(resulting_category);
		execute_query(stmt,query);
		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET resulting_category=%3 "+
						"WHERE type=%1 AND resulting_category=%2")
			  .arg((int)rule).arg(category_id).arg(resulting_category);
		execute_query(stmt,query);
		//assert(query.numRowsAffected()>0);

	}
	loadAffixList();
	errors->setText(*errors_text);
}

void SplitDialog::removeStaleCategoriesAndAffixes() {
	item_types type=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(type==PREFIX?AA:CC);
	Retrieve_Template t("compatibility_rules","category_id1","category_id2","resulting_category", tr("type=%1").arg(rule));
	while (t.retrieve()) {
		long category_id1=t.get(0).toLongLong();
		long category_id2=t.get(1).toLongLong();
		long resulting_category=t.get(2).toLongLong();
		Search_Compatibility s(rule,resulting_category);
		Search_by_category s2(category_id2);
		if ((s.size()==0 && !isAcceptState(type,resulting_category)) || s2.size()==0) {
			QString cat1=database_info.comp_rules->getCategoryName(category_id1);
			QString cat2=database_info.comp_rules->getCategoryName(category_id2);
			QString catr=database_info.comp_rules->getCategoryName(resulting_category);
			warning<<"Removing Rule ("<<cat1<<","<<cat2<<","<<catr<<")\n";

			QString stmt=QString(tr("DELETE  ")+
									"FROM  compatibility_rules "+
									"WHERE type=%1 AND (category_id1=%2 AND category_id2=%3)")
						  .arg((int)rule).arg(category_id1).arg(category_id2);
			QSqlQuery query(db);
			execute_query(stmt,query);
			assert(query.numRowsAffected()==1);

		}
	}
	int rowCount=originalAffixList->rowCount();
	for (int i=0;i<rowCount;i++) {
		long affix_id=originalAffixList->item(i,0)->text().toLongLong();
		//QString affix=originalAffixList->item(i,1)->text();
		QString category=originalAffixList->item(i,2)->text();
		QString raw_data=originalAffixList->item(i,3)->text();
		QString pos=originalAffixList->item(i,4)->text();
		QString description=originalAffixList->item(i,5)->text();
		long cat_id=database_info.comp_rules->getCategoryID(category);
		if (!isAcceptState(type,cat_id)) {
			Search_Compatibility s(rule,cat_id);
			Search_Compatibility s2(rule,cat_id,false);
			if (s.size()==0 && s2.size()==0 ) {
				long description_id=database_info.descriptions->indexOf(description);
				remove_item(type,affix_id,raw_data,cat_id,description_id,pos);
			}
		}
	}
	loadAffixList();
	errors->setText(*errors_text);
}

void SplitDialog::mergeSimilarCategories() {
	item_types type=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(type==PREFIX?AA:CC);
	Retrieve_Template t("compatibility_rules","category_id1","category_id2","resulting_category", tr("type=%1").arg(rule));
	while (t.retrieve()) {
		long category_id1_1=t.get(0).toLongLong();
		long category_id2=t.get(1).toLongLong();
		long resulting_category=t.get(2).toLongLong();
		Retrieve_Template t2("compatibility_rules","category_id1",
							tr("type=%1 AND category_id2=%2 AND resulting_category=%3 AND category_id1<>%4").arg(rule).arg(category_id2).arg(resulting_category).arg(category_id1_1));
		while (t2.retrieve()) {
			long category_id1_2=t2.get(0).toLongLong();
			bool stop=false;
			for (int i=0;i<2;i++) {
				bool first=(i==0); //so that to do one check for category1 and other for category2
				Search_Compatibility c1(rule,category_id1_1,first);
				long cat2_1,cat2_2,result1,result2;
				while (c1.retrieve(cat2_1,result1)) {
					Search_Compatibility c2(rule,category_id1_2,first);
					if (c1.size()!=c2.size()) {
						stop=true;
						break;
					}
					bool found=false;
					while (c2.retrieve(cat2_2,result2)) {
						if (cat2_1==cat2_2 && result1==result2) {
							found=true;
							break;
						}
					}
					if (!found) {
						stop=true;
						break;
					}
				}
				if (stop)
					break;
			}
			if (stop)
				continue;
			QMessageBox msgBox;
			QString cat1=database_info.comp_rules->getCategoryName(category_id1_1),
					cat2=database_info.comp_rules->getCategoryName(category_id1_2);
			msgBox.setText(tr("About to merge ")+cat2+","+cat1+"\n");
			msgBox.setInformativeText("Do you want to add it?");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);
			int ret = msgBox.exec();
			if (ret==QMessageBox::Yes) {
				warning << QString("Merged the 2 categories: %1, %2 into %1\n").arg(cat1).arg(cat2);
				QSqlQuery query(db);
				QString stmt=QString(tr("UPDATE %1_category ")+
										"SET category_id=%2 "+
										"WHERE category_id=%3")
							  .arg(interpret_type(type)).arg(category_id1_1).arg(category_id1_2);

				execute_query(stmt,query);
				//assert(query.numRowsAffected()>0);
				stmt=QString(tr("DELETE  ")+
								"FROM  compatibility_rules "+
								"WHERE type=%1 AND (category_id1=%2 OR category_id2=%2)")
							  .arg((int)rule).arg(category_id1_2);
				execute_query(stmt,query);
				//assert(query.numRowsAffected()>0);
				return mergeSimilarCategories();
			}
		}

	}
	loadAffixList();
	errors->setText(*errors_text);
}

void SplitDialog::removeDummyRulesForConsistencyIfNotNeeded() {
	item_types type=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(type==PREFIX?AA:CC);
	QSqlQuery query(db);
	QString stmt=QString(tr("SELECT t1.category_id2, t2.category_id2  ")+
					"FROM  compatibility_rules t1, compatibility_rules t2 "+
					"WHERE t1.type=%1 AND t2.type=%1 AND t1.category_id1=%2 AND t2.category_id1=%2 AND t1.resulting_category=t2.resulting_category AND t1.category_id2<t2.category_id2")
				  .arg((int)rule).arg(cat_empty);
	execute_query(stmt,query);
	while(query.next()) {
		long id1=query.value(0).toULongLong();
		long id2=query.value(1).toULongLong();
		/*if (id1!=13 && id2!=66)
			continue;*/
		long idMain, idOld;
		if (isAcceptState(type,id1)) {
			assert(!isAcceptState(type,id2));
			idMain=id1;
			idOld=id2;
		} else if (isAcceptState(type,id2)) {
			idMain=id2;
			idOld=id1;
		}
		QString catMain=database_info.comp_rules->getCategoryName(idMain),
				catOld=database_info.comp_rules->getCategoryName(idOld);
		warning << QString("Revert the 2 categories: %1, %2 into %1\n").arg(catMain).arg(catOld);
		QSqlQuery query(db);
		QString stmt=QString(tr("UPDATE %1_category ")+
								"SET category_id=%2 "+
								"WHERE category_id=%3")
					  .arg(interpret_type(type)).arg(idMain).arg(idOld);
		execute_query(stmt,query);
		//assert(query.numRowsAffected()>0);

		stmt=QString(tr("DELETE  ")+
						"FROM  compatibility_rules "+
						"WHERE type=%1 AND (category_id1=%2 AND category_id2=%3)")
					  .arg((int)rule).arg(cat_empty).arg(idOld);
		execute_query(stmt,query);

		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET category_id1=%2 "+
						"WHERE type=%1 AND category_id1=%3")
					  .arg(interpret_type(type)).arg(idMain).arg(idOld);
		execute_query(stmt,query);
		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET category_id2=%2 "+
						"WHERE type=%1 AND category_id2=%3")
					  .arg(interpret_type(type)).arg(idMain).arg(idOld);
		execute_query(stmt,query);
		stmt=QString(tr("UPDATE compatibility_rules ")+
						"SET resulting_category=%2 "+
						"WHERE type=%1 AND resulting_category=%3")
					  .arg(interpret_type(type)).arg(idMain).arg(idOld);
		execute_query(stmt,query);
	}
}

void SplitDialog::split_action() {
	QList<QTableWidgetSelectionRange>  selection=originalAffixList->selectedRanges();
	item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	if (selection.size()==1 && selection[0].rowCount()==1){
		if (source_id<0)
			source_id=insert_source("Manual Work by Jad", "Splitting Affixes by an automatic process","Jad Makhlouta");
		//select the items to be splitted
		originalAffixList->setSelectionMode(QAbstractItemView::MultiSelection);
		int rowIndex=selection[0].topRow();
		//long affix_id=originalAffixList->item(rowIndex,0)->text().toLongLong();
		QString affix=originalAffixList->item(rowIndex,1)->text();
		QString category=originalAffixList->item(rowIndex,2)->text();
		QString raw_data=originalAffixList->item(rowIndex,3)->text();
		QString pos=originalAffixList->item(rowIndex,4)->text();
		QString description=originalAffixList->item(rowIndex,5)->text();
		originalAffixList->clearSelection();
		int rowCount=originalAffixList->rowCount();
		for (int i=0;i<rowCount;i++) {
			long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
			QString affix1=originalAffixList->item(i,1)->text();
			QString category1=originalAffixList->item(i,2)->text();
			QString raw_data1=originalAffixList->item(i,3)->text();
			QString pos1=originalAffixList->item(i,4)->text();
			QString description1=originalAffixList->item(i,5)->text();
			bool canBeEmpty=description.startsWith('[') && description.endsWith(']') && pos.count('+')==1;
			if (	affix1.startsWith(affix) &&
					raw_data1.startsWith(raw_data) &&
					pos1.startsWith(pos) &&
					(description1.startsWith(description) || description1.endsWith(description) ||
					 canBeEmpty)	) {
				bool reverse_description=true;
				if (description1.startsWith(description))
					reverse_description=false;
				originalAffixList->selectRow(i);
				//split the item selected
				QString affix2=affix1.mid(affix.size());
				QString raw_data2=raw_data1.mid(raw_data.size());
				QString pos2=pos1.mid(pos.size());
			#ifdef SAMA
				if (pos1.size()>pos.size() && pos1.at(pos.size())=='+')
					pos2=pos1.mid(pos.size()+1);
			#endif
				QString description2;
				if (canBeEmpty && !description1.startsWith(description))
					description2=description1;
				else if (!reverse_description)
					description2=description1.mid((description.size()==0?-1:description.size())+ (t==PREFIX?3:1)); //' + ' or ' '
				else
					description2=description1.mid(0,description1.size()-(description.size()==0?-1:description.size())- (t==PREFIX?3:1)); //' '
				int rowIndex2=getRow(affix2,raw_data2,pos2,description2);
				if (rowIndex2<0) {
					if (!pos2.isEmpty())
						warning<<"("<<affix2<<","<<raw_data2<<","<<description2<<","<<pos2<<") not found.\n";
					continue;
				} else {
					bool reverse_description2=(bool)originalAffixList->item(i,6)->text().toInt();
					if (reverse_description2 && ! reverse_description){
						error<<"Inconsistent Reverse Description.\n";
						continue;
					}
				#if 0 //seems wrong logic
					if (reverse_description){
						originalAffixList->setItem(rowIndex2,6,new QTableWidgetItem(QString("1")));
						QString category2=originalAffixList->item(rowIndex2,2)->text();
						warning<<"updating reverse_description...\n";
						KEEP_OLD=false;
						insert_item(t,affix2,raw_data2,category2,source_id,new QList<long>,description2,pos2,"","1");
					}
				#endif
				}
				QString category2=originalAffixList->item(rowIndex2,2)->text();
				long category_id1=database_info.comp_rules->getCategoryID(category1);
				remove_item(t,affix_id1,raw_data1,category_id1,database_info.descriptions->indexOf(description1),pos1);

				//in case this category has been renamed by some number in differentiate categories
				long cat_result;
				if ((*database_info.comp_rules)(cat_empty,category_id1,cat_result)) {
					category_id1=cat_result; //not needed
					category1=database_info.comp_rules->getCategoryName(cat_result);
				}

				insert_compatibility_rules((t==PREFIX?AA:CC),category,category2,category1,source_id);
				int compRowIndex=compatRulesList->rowCount();
				compatRulesList->setRowCount(compRowIndex+1);
				compatRulesList->setItem(compRowIndex,0,new QTableWidgetItem(category));
				compatRulesList->setItem(compRowIndex,1,new QTableWidgetItem(category2));
				compatRulesList->setItem(compRowIndex,2,new QTableWidgetItem(category1));
			}
		}
	}
	loadAffixList();
	errors->setText(*errors_text);
}

void SplitDialog::reverse_action() {
	item_types type=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(type==PREFIX?AA:CC);
	Retrieve_Template t("compatibility_rules","category_id1","category_id2","resulting_category", tr("type=%1").arg(rule));
	while (t.retrieve())
	{
		long category_id2=t.get(0).toLongLong();
		long category_id3=t.get(1).toLongLong();
		long resulting_category1=t.get(2).toLongLong();
		Search_Compatibility s(rule,resulting_category1,false);
		long category_id1,resulting_category2;
		while (s.retrieve(category_id1,resulting_category2))
		{
			if (!(category_id2==category_id1 && resulting_category1==category_id3))
			{
				assert(category_id1!=category_id2);
				if (!areCompatible(rule,category_id1,category_id2))
				{
					QMessageBox msgBox;
					msgBox.setText(tr("About to add Rule (")+database_info.comp_rules->getCategoryName(category_id1)+","+database_info.comp_rules->getCategoryName(category_id2)+")\n"
								   +"\tcategory3="+database_info.comp_rules->getCategoryName(category_id3)+",resulting1="+database_info.comp_rules->getCategoryName(resulting_category1)+",resulting2="+database_info.comp_rules->getCategoryName(resulting_category2));
					msgBox.setInformativeText("Do you want to add it?");
					msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
					msgBox.setDefaultButton(QMessageBox::Yes);
					int ret = msgBox.exec();
					if (ret==QMessageBox::Yes)
					{
						insert_compatibility_rules(rule,category_id1,category_id2,source_id);
						warning<<"Added Rule ("<<database_info.comp_rules->getCategoryName(category_id1)<<","<<database_info.comp_rules->getCategoryName(category_id2)<<")\n"
								<<"\tcategory3="<<database_info.comp_rules->getCategoryName(category_id3)<<",resulting1="<<database_info.comp_rules->getCategoryName(resulting_category1)<<",resulting2="<<database_info.comp_rules->getCategoryName(resulting_category2)<<"\n";
					}
				}
			}
		}
	}
}

void SplitDialog::specializeAllDuplicateEntries() {
	item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	typedef QMap<QString,int> NumberMap;
	NumberMap map;
	int rowCount=originalAffixList->rowCount();
	for (int i=0;i<rowCount;i++) {
		//long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
		QString category=originalAffixList->item(i,2)->text();
		long category_id=getID("category",category,QString("type=%1").arg((int)t));
		if (cat_empty!=category_id && isAcceptState(t,category_id)) {
			NumberMap::iterator itr=map.find(category);
			QString category_changed;
			if (itr!=map.end()) {
				int i=++(itr.value());
				category_changed=(category+"_%1").arg(i);
			} else {
				map[category]=0;
				category_changed=category+"_0";
			}
			renameCategory(i,category,category_changed);
		}
	}
	system(QString("rm ").append(compatibility_rules_path).toStdString().data());
	database_info.comp_rules->buildFromFile();
	loadAffixList();
	errors->setText(*errors_text);
}

void SplitDialog::renameCategory(int row,QString category_original, QString category_specialized) {
	originalAffixList->selectRow(row);
	specializeHelper(category_original,category_specialized,"",false);
}

void SplitDialog::specializeHelper(QString category_original, QString category_specialized, QString category_left, bool modifyLeftEntries) {
	item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
	rules rule=(t==PREFIX?AA:CC);
	long cat_org_id=getID("category",category_original,QString("type=%1").arg((int)t));
	long specialized_id=insert_category(category_specialized,t,source_id,false);
	long left_id=insert_category(category_left,t,source_id,false);
	Search_Compatibility s(rule,cat_org_id,true);
	long cat_id2,cat_result;
	while (s.retrieve(cat_id2,cat_result)) {
		insert_compatibility_rules(rule,specialized_id,cat_id2,cat_result,source_id);
		insert_compatibility_rules(rule,left_id,cat_id2,cat_result,source_id);
	}
	Search_Compatibility s2(rule,cat_org_id,false);
	while (s2.retrieve(cat_id2,cat_result)) {
		insert_compatibility_rules(rule,cat_id2,specialized_id,cat_result,source_id);
		insert_compatibility_rules(rule,cat_id2,left_id,cat_result,source_id);
	}

	//second change categories of those selected
	QSqlQuery query(db);
	QList<QTableWidgetSelectionRange>  selection=originalAffixList->selectedRanges();
	for (int i=0;i<selection.size();i++) {
		int top=selection[i].topRow();
		for (int j=0;j<selection[i].rowCount();j++) {
			int rowIndex=top+j;
			long affix_id=originalAffixList->item(rowIndex,0)->text().toLongLong();
			QString raw_data=originalAffixList->item(rowIndex,3)->text();
			QString pos=originalAffixList->item(rowIndex,4)->text();
			QString description=originalAffixList->item(rowIndex,5)->text();
			QString stmt=QString(tr("UPDATE %1_category ")+
								  "SET category_id=%2 "+
								  "WHERE %1_id=%3 AND category_id=%4 AND raw_data=\"%5\" AND POS=\"%6\" AND "+
										"description_id IN (SELECT id FROM description WHERE name =\"%7\" AND type=%8)")
						  .arg(interpret_type(t)).arg(specialized_id).arg(affix_id).arg(cat_org_id).arg(raw_data)
						  .arg(pos).arg(description).arg((int)t);
			execute_query(stmt,query);
			int num=query.numRowsAffected();
			assert (num==1 || num==-1);
		}
	}
	//third: add 2 dummy rules to keep consistency with AB,AC,BC
	insert_compatibility_rules(rule,cat_empty,specialized_id,cat_org_id,source_id);
	if (modifyLeftEntries) {
		//forth: change category of those not selected
		execute_query(QString(tr( "UPDATE %1_category ")+
								  "SET category_id=%2 "+
								  "WHERE category_id=%3")
					  .arg(interpret_type(t)).arg(left_id).arg(cat_org_id),query);
		if (query.numRowsAffected()==0)
			warning << "Operation performed performed just renaming because there are no left entries of this query\n";
		if (left_id!=cat_org_id)
			insert_compatibility_rules(rule,cat_empty,left_id,cat_org_id,source_id);
		//fifth: remove all rules AA or CC having the old_category if (old not used again)
		if (category_original!=category_left) {
			QString stmt=QString(tr("DELETE  ")+
								  "FROM  compatibility_rules "+
								  "WHERE type=%1 AND (category_id1=%2 OR category_id2=%2)")
						  .arg((int)rule).arg(cat_org_id);
			execute_query(stmt,query);
		}
	}
}

void SplitDialog::specialize_action(){
	if (originalAffixList->selectionMode()==QAbstractItemView::MultiSelection) {
		QList<QTableWidgetSelectionRange>  selection=originalAffixList->selectedRanges();
		if (selection.size()>=1){
			QString category_original="";
			for (int i=0;i<selection.size();i++) {
				int top=selection[i].topRow();
				for (int j=0;j<selection[i].rowCount();j++) {
					int rowIndex=top+j;
					if (category_original=="")
						category_original=originalAffixList->item(rowIndex,2)->text();
					else if (category_original!=originalAffixList->item(rowIndex,2)->text()) {
						error<<"Operation Failed: because not all selected rows have same category\n";
						return;
					}
				}
			}
			//first add category of new Names for the specialized category and category of those left and duplicate previous rules to them
			bool ok;
			QString category_specialized = category_original;
			while (category_specialized==category_original) {
				category_specialized=QInputDialog::getText(this, tr("Name for Scpecialized Category"),
									 tr("New Name: (must be different from original)"), QLineEdit::Normal,
									 category_original, &ok);
			}
			QString category_left = QInputDialog::getText(this, tr("Name of Category for entries left"),
												  tr("New Name:"), QLineEdit::Normal,
												  category_original, &ok);

			if (ok && !category_specialized.isEmpty() && !category_left.isEmpty()) {
				specializeHelper(category_original,category_specialized,category_left);
				system(QString("rm ").append(compatibility_rules_path).toStdString().data());
				database_info.comp_rules->buildFromFile();
				loadAffixList();
			}
		}
	}else {
		originalAffixList->setSelectionMode(QAbstractItemView::MultiSelection);
	}
}

void splitRecursiveAffixes(){

	SplitDialog * d=new SplitDialog();
	d->exec();
}
