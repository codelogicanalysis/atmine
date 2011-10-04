#include "bibleManualTagger.h"
#include <QtAlgorithms>

void BibleTaggerDialog::tagGenealogy_action() {
	if (this==NULL)
		return;
	moveSelectionToSentenceBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
	QList<int> listForRemoval;
	Selection * sel=NULL;
	while (i>=0) {
		if (sel==NULL) {
			sel=&tags[i];
		} else {
			GeneTree * duplicate=sel->tree->duplicateTree();
			duplicate->mergeTrees(tags[i].tree);
			Selection::MainSelectionList mergedNames;
			Selection::mergeNames(string,sel->names,tags[i].names,mergedNames);
			int count=duplicate->getTreeNodesCount(true);
			if (count==mergedNames.size()) {
				listForRemoval.append(i);
				sel->names=mergedNames;
				sel->tree->deleteTree();
				tags[i].tree->deleteTree();
				sel->tree=duplicate;
				sel->setMainInterval(min(sel->getMainStart(),tags[i].getMainStart()),max(sel->getMainEnd(),tags[i].getMainEnd()));
				tags.removeAt(i);
				i--;
			} else {
				duplicate->deleteTree(); //stopped before completion i>=0
				break;
			}
		}
		i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
	}
	if (sel==NULL) {
		tags.append(Selection(string,start,end));
	} else if (i>=0) { //stopped before completion
		start=sel->getMainStart();
		end=sel->getMainEnd();
	} else {
		sel->setMainInterval(min(sel->getMainStart(),start),max(sel->getMainEnd(),end-1));
	}
	c.setPosition(start,QTextCursor::MoveAnchor);
	c.setPosition(end,QTextCursor::KeepAnchor);
	text->setTextCursor(c);
	text->setTextBackgroundColor(Qt::darkYellow);
	c.clearSelection();
	text->setTextCursor(c);
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::unTagGenealogy_action() {
	if (this==NULL)
		return;
	Selection::MainSelectionList listForRemoval;
	int i=findSelection(0,SELECTION_OUTSIDE);
	if (i>=0) {
		while (i>=0) {
			listForRemoval.append(Selection::MainSelection(tags[i].getMainStart(),tags[i].getMainEnd()));
			tags.removeAt(i);
			i--;
			i=findSelection(i+1,SELECTION_OUTSIDE);
		}
	} else {
		QTextCursor c=text->textCursor();
		int start=c.selectionStart();
		int end=c.selectionEnd();
		int i=findSelection(0,SELECTION_OUTSIDEOVERLAP);
		if (i>=0) {
			listForRemoval.append(Selection::MainSelection(start,end-1));
		}
		while (i>=0) {
			if (tags[i].getMainStart()>=start) {
				tags[i].setMainInterval(end,tags[i].getMainEnd());
			} else {
				tags[i].setMainInterval(tags[i].getMainStart(),start-1);
			}
			if (tags[i].getMainStart()==tags[i].getMainEnd()) {
				tags.removeAt(i);
				i--;
			} else
				tags[i].removeExtraNames();
			i=findSelection(i+1,SELECTION_OUTSIDEOVERLAP);
		}
	}
	QTextCursor c=text->textCursor();
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second+1,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1.tags").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags
			<<*globalGraph;
		file.close();
		QFile::remove(file.fileName()+".copy");
		file.copy(file.fileName()+".copy");
	} else
		error << "Unexpected Error: Unable to open file\n";
}

void BibleTaggerDialog::open_action() {
	QFile fileOriginal(QString(filename).toStdString().data());
	if (fileOriginal.open(QIODevice::ReadOnly))	{
		text->clear();
		QTextStream s(&fileOriginal);
		s.setCodec("utf-8");
		text->setLayoutDirection(Qt::RightToLeft);
		QTextCursor c=text->textCursor();
		c.clearSelection();
		c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		string=new QString(s.readAll());
		text->setText(*string);

		QFile file(QString("%1.tags").arg(filename).toStdString().data());
		if (file.open(QIODevice::ReadOnly))	{
			QDataStream out(&file);   // we will serialize the data into the file
			out	>> tags;
			if (!out.atEnd()) {
				globalGraph=new GeneTree();
				out	>>*globalGraph;
			}
			file.close();
			for (int i=0;i<tags.size();i++) {
				tags[i].text=string;
				FillTextVisitor v(string);
				v(tags[i].getTree());
				QTextCursor c=text->textCursor();
				int start=tags[i].getMainStart();
				int end=tags[i].getMainEnd();
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(end+1,QTextCursor::KeepAnchor);
				text->setTextCursor(c);
				text->setTextBackgroundColor(Qt::darkYellow);
				const Selection::MainSelectionList & names=tags[i].getNamesList();
				for (int i=0;i<names.size();i++) {
					int start=names[i].first;
					int end=names[i].second;
					c.setPosition(start,QTextCursor::MoveAnchor);
					c.setPosition(end+1,QTextCursor::KeepAnchor);
					text->setTextCursor(c);
					text->setTextColor(Qt::white);
				}
			}
			c.clearSelection();
			c.setPosition(0);
			text->setTextCursor(c);

		} else {
			error << "Annotation File does not exist\n";
		}
	} else {
		error << "File does not exist\n";
	}
}

void BibleTaggerDialog::unTagName_action() {
	if (this==NULL)
		return;
	QTextCursor c=text->textCursor();
	int i=findSelection(0,SELECTION_INSIDE);
	Selection::MainSelectionList listForRemoval;
	while (i>=0) { //while not useful
		const Selection::MainSelectionList & names=tags[i].getNamesList();
		int j=findSubSelection(i,0);
		while (j>=0) {
			listForRemoval.append(Selection::MainSelection(names[j].first,names[j].second));
			tags[i].removeNameAt(j);
			j--;
			j=findSubSelection(i,j+1);
		}
		i=findSelection(i+1,SELECTION_INSIDE);
	}
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second+1,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::tagName_action() {
	if (this==NULL)
		return;
	if (forceWordNames->isChecked())
		moveSelectionToWordBoundaries();
	QTextCursor c=text->textCursor();
	int start=c.selectionStart();
	int end=c.selectionEnd();
	if (start==end)
		return;
	int i=findSelection(0,SELECTION_INSIDE);
	if (i>=0) {
		int j=findSubSelection(0);
		if (j<0) {
			tags[i].addName(start,end-1);
			text->setTextColor(Qt::white);
			c.clearSelection();
			text->setTextCursor(c);
		}
	}
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::text_selectionChangedAction() {
	int i=findSelection(0,SELECTION_INSIDE);
	if (selectedTagIndex==i)
		return;
	if (isGlobalGraph->isChecked())
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::updateGraphDisplay() {
	if (!isGlobalGraph->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		if (i>=0) {
			treeText->setText(tags[i].getText());
			treeText->setReadOnly(false);
			modifyGraph->setEnabled(true);
			tags[i].getTree()->displayTree(this);
			selectedTagIndex=i;
		} else {
			treeText->setText("");
			treeText->setReadOnly(true);
			modifyGraph->setEnabled(false);
			graph->clear();
			selectedTagIndex=-1;
		}
	} else {
		treeText->setText(Selection::getTreeText(globalGraph));
		treeText->setReadOnly(false);
		modifyGraph->setEnabled(true);
		globalGraph->displayTree(this);
		selectedTagIndex=-1;
	}
}

void BibleTaggerDialog::modifyGraph_action() {
	if (!isGlobalGraph->isChecked()) {
		int i=findSelection(0,SELECTION_INSIDE);
		assert(i>=0);
		if (tags[i].updateGraph(treeText->toPlainText()))
			tags[i].getTree()->displayTree(this);
	} else {
		if (Selection::updateGraph(treeText->toPlainText(),NULL,&globalGraph,string,this))
			globalGraph->displayTree(this);
	}
}

void BibleTaggerDialog::isGlobalGraph_action(){
	updateGraphDisplay();
	resetGlobalGraph->setEnabled(isGlobalGraph->isChecked());
}

int bibleTagger(QString input_str){
	BibleTaggerDialog * d=new BibleTaggerDialog(input_str);
	d->show();
	return 0;
}
