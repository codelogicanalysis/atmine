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
	text->setTextBackgroundColor(Qt::darkYellow);
	c.clearSelection();
	text->setTextCursor(c);
	if (end>start)
		tags.append(Selection(string,start,end));
	updateGraphDisplay();
}

void BibleTaggerDialog::unTagGenealogy_action() {
	if (this==NULL)
		return;
	int i=findSelection(0);
	while (i>=0) {
		QTextCursor c=text->textCursor();
		c.setPosition(tags[i].getMainStart(),QTextCursor::MoveAnchor);
		c.setPosition(tags[i].getMainEnd(),QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextBackgroundColor(Qt::white);
		text->setTextColor(Qt::black);
		c.clearSelection();
		text->setTextCursor(c);
		tags.removeAt(i);
		i--;
		i=findSelection(i+1);
	}
	updateGraphDisplay();
}

void BibleTaggerDialog::save_action() {
	qSort(tags.begin(),tags.end());
	QFile file(QString("%1.tags").arg(filename).toStdString().data());
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);   // we will serialize the data into the file
		out	<< tags;
		file.close();
	} else
		error << "Unexpected Error: Unable to open file\n";
}

class FillTextVisitor:public GeneVisitor {
private:
	QString * text;
public:
	FillTextVisitor(QString * text) {
		this->text=text;
	}
	void visit(const GeneNode * node,int ) {
		 ((GeneNode *)node)->name.text=text;
	}
	void visit(const GeneNode *, const Name & name,bool isSpouse) {
		if (isSpouse)
			((Name &)name).text=text;
	}
	void finish() {}
};

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
			file.close();
			for (int i=0;i<tags.size();i++) {
				tags[i].text=string;
				FillTextVisitor v(string);
				v(tags[i].getTree());
				QTextCursor c=text->textCursor();
				int start=tags[i].getMainStart();
				int end=tags[i].getMainEnd();
				c.setPosition(start,QTextCursor::MoveAnchor);
				c.setPosition(end,QTextCursor::KeepAnchor);
				text->setTextCursor(c);
				text->setTextBackgroundColor(Qt::darkYellow);
				const Selection::MainSelectionList & names=tags[i].getNamesList();
				for (int i=0;i<names.size();i++) {
					int start=names[i].first;
					int end=names[i].second;
					c.setPosition(start,QTextCursor::MoveAnchor);
					c.setPosition(end,QTextCursor::KeepAnchor);
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
	int i=findSelection(0,true);
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
		i=findSelection(i+1,true);
	}
	for (int i=0;i<listForRemoval.size();i++) {
		c.setPosition(listForRemoval[i].first,QTextCursor::MoveAnchor);
		c.setPosition(listForRemoval[i].second,QTextCursor::KeepAnchor);
		text->setTextCursor(c);
		text->setTextColor(Qt::black);
		text->setTextCursor(c);
	}
	c.clearSelection();
	text->setTextCursor(c);
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
	int i=findSelection(0,true);
	if (i>=0) {
		int j=findSubSelection(0);
		if (j<0) {
			tags[i].addName(start,end);
			text->setTextColor(Qt::white);
			c.clearSelection();
			text->setTextCursor(c);
		}
	}
	updateGraphDisplay();
}

void BibleTaggerDialog::text_selectionChangedAction() {
	int i=findSelection(0,true);
	if (selectedTagIndex==i)
		return;
	updateGraphDisplay();
}

void BibleTaggerDialog::updateGraphDisplay() {
	int i=findSelection(0,true);
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
}

void BibleTaggerDialog::modifyGraph_action() {
	int i=findSelection(0,true);
	assert(i>=0);
	if (tags[i].updateGraph(treeText->toPlainText()))
		tags[i].getTree()->displayTree(this);
}

int bibleTagger(QString input_str){
	BibleTaggerDialog * d=new BibleTaggerDialog(input_str);
	d->show();
	return 0;
}
