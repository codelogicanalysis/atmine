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
					description1.startsWith(description)	) {
				originalAffixList->selectRow(i);
				//split the item selected
				QString affix2=affix1.mid(affix.size());
				QString raw_data2=raw_data1.mid(raw_data.size());
				QString pos2=pos1.mid(pos.size());
				QString description2=description1.mid(description.size()+ (t==PREFIX?3:1)); //' + ' or '+'
				int rowIndex2=getRow(affix2,raw_data2,pos2,description2);
				if (rowIndex2<0) {
					if (!pos2.isEmpty())
						warning<<"("<<affix2<<","<<raw_data2<<","<<pos2<<","<<description2<<") not found.\n";
					continue;
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


void splitRecursiveAffixes(){

	SplitDialog * d=new SplitDialog();
	d->exec();
}
