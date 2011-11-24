#include "split_affixes.h"


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
			if (	affix1.startsWith(affix) &&
					raw_data1.startsWith(raw_data) &&
					pos1.startsWith(pos) &&
					(description1.startsWith(description) || description1.endsWith(description) )	) {
				bool reverse_description=true;
				if (description1.startsWith(description))
					reverse_description=false;
				originalAffixList->selectRow(i);
				//split the item selected
				QString affix2=affix1.mid(affix.size());
				QString raw_data2=raw_data1.mid(raw_data.size());
				QString pos2=pos1.mid(pos.size());
				QString description2;
				if (!reverse_description)
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
				remove_item(t,affix_id1,raw_data1,database_info.comp_rules->getCategoryID(category1),database_info.descriptions->indexOf(description1),pos1);

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
		Search_Compatibility s(AA,resulting_category1,false);
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

void splitRecursiveAffixes(){

	SplitDialog * d=new SplitDialog();
	d->exec();
}

void SplitDialog::specialize_action(){
	if (originalAffixList->selectionMode()==QAbstractItemView::MultiSelection) {
		QList<QTableWidgetSelectionRange>  selection=originalAffixList->selectedRanges();
		item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
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

				//second change categories of those selected and those non-selected
				QSqlQuery query(db);
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
				execute_query(QString(tr("UPDATE %1_category ")+
									  "SET category_id=%2 "+
									  "WHERE category_id=%3")
							  .arg(interpret_type(t)).arg(left_id).arg(cat_org_id),query);
				if (query.numRowsAffected()==0)
					warning << "Operation performed performed just renaming because there are no left entries of this query\n";
				//third: add 2 dummy rules to keep consistency with AB,AC,BC
				insert_compatibility_rules(rule,cat_empty,specialized_id,cat_org_id,source_id);
				insert_compatibility_rules(rule,cat_empty,left_id,cat_org_id,source_id);
				//forth: remove all rules AA or CC having the old_category if (old not used again)
				if (category_original!=category_left) {
					QString stmt=QString(tr("DELETE  ")+
										  "FROM  compatibility_rules "+
										  "WHERE type=%1 AND (category_id1=%2 OR category_id2=%2)")
								  .arg((int)rule).arg(cat_org_id);
					execute_query(stmt,query);
				}
				system(QString("rm ").append(compatibility_rules_path).toStdString().data());
				database_info.comp_rules->buildFromFile();
				loadAffixList();
			}
		}
	}else {
		originalAffixList->setSelectionMode(QAbstractItemView::MultiSelection);
	}
}
