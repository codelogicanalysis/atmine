#include <QMessageBox>
#include "hadithChainGraph.h"
#include "twoLevelTaggerSelection.h"
#include "hadithChainItemModel.h"
#include "bibleGeneology.h"

#define DELETE_NARRATORS

HadithChainGraph::HadithChainGraph():chain(NULL) { }

HadithChainGraph::HadithChainGraph(Chain & c):chain(c) {}

AbstractGraph * HadithChainGraph::readFromStreamHelper(QDataStream &in) {
	in>>chain;
	return this;
}
void HadithChainGraph::writeToStream(QDataStream &out) {
	out<<chain;
}
AbstractGraph * HadithChainGraph::duplicate() {
	HadithChainGraph *c=new HadithChainGraph();
	c->chain=chain;
	return c;
}
AbstractGraph * HadithChainGraph::merge(AbstractGraph *graph2) {
	return this;
}
bool HadithChainGraph::isRepresentativeOf(const MainSelectionList &list) {
	return list.size()==chain.m_chain.size();
}
QString HadithChainGraph::getText() {
	QString text;
	for (int i=chain.m_chain.size()-1;i>=0;i--)
		text+="<"+chain.m_chain[i]->getString()+">"+"\n";
	return text;
}

int HadithChainGraph::read(ChainsContainer & chains, ATMProgressIFC * prg, QString) {
	NarratorReader * this_=(NarratorReader *)prg;
	int size=chains.size();
	for (int i=0;i<size;i++) {
		Chain & c=*chains[i];
		int chainSize=c.m_chain.size();
		for (int j=0;j<chainSize;j++) {
			ChainPrim * chainPrim=c.m_chain[j];
			if (chainPrim->isNarrator()) {
				Narrator * n=dynamic_cast<Narrator*>(chainPrim);
				if (this_->narr==NULL) {
					this_->narr=n;
				} else {
					if (this_->displayWarnings) {
						QMessageBox msgBox;
						msgBox.setIcon(QMessageBox::Information);
						msgBox.setWindowTitle("Warning Processing Graph");
						msgBox.setText(QString("More than one narrator was found in supposedly one text. The others will be ignored"));
						msgBox.exec();
					}
					break;
				}
			}
		}
	}
	return 0;
}

Narrator * HadithChainGraph::getNarrator(QString &text, bool displayWarnings) {
	Name n(&text,0,text.size()-1);
	return getNarrator(n,displayWarnings);
}

Narrator * HadithChainGraph::getNarrator(const Name & name,bool displayWarnings) {
	NarratorReader r;
	r.displayWarnings=displayWarnings;
	segmentNarrators(name.getTextPointer(),name.getStart(),name.getEnd(),&read,&r);
	bool addEdges=false;
	if (r.narr==NULL) {
		addEdges=true;
		r.narr=new Narrator(name.getTextPointer());
		NamePrim * n=new NamePrim(name.getTextPointer(),name.getStart());
		n->m_end=name.getEnd();
		r.narr->m_narrator.append(n);
	} else {
		NameConnectorPrim * con;
		if (addEdges || name.getStart()<r.narr->getStart()) {
			con=new NameConnectorPrim(name.getTextPointer(),name.getStart());
			con->m_end=name.getStart();
			r.narr->m_narrator.prepend(con);
		}
		if (addEdges || name.getEnd()>r.narr->getEnd()) {
			con=new NameConnectorPrim(name.getTextPointer(),name.getEnd());
			con->m_end=name.getEnd();
			r.narr->m_narrator.append(con);
		}
	}
	return r.narr;
}

bool HadithChainGraph::buildFromText(QString text, TwoLevelSelection *sel, QString */*string*/, AbstractTwoLevelAnnotator */*annotator*/) {
	QStringList lines=text.split(QRegExp("[<>]"),QString::SkipEmptyParts);
	for (int i=0;i<lines.size();i++) {
		if (lines[i]=="\n") {
			lines.removeAt(i);
			i--;
		}
	}
	QString s;
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("Error Processing Tree");
	HadithChainGraph * newGraph=new HadithChainGraph();
	foreach(s,lines) {
		Name name(sel->getTextPointer(),-1,-1);
		for (int i=sel->getNamesList().size()-1;i>=0;i--) {
			Name n=Name(sel->getTextPointer(),sel->getNamesList().at(i).first,sel->getNamesList().at(i).second);
			if (equal_ignore_diacritics(n.getString(),s)) {
				name=n;
				break;
			}
		}
		if (name.getStart()<0) {
			msgBox.setText(QString("Narrator \"%1\" not found").arg(s));
			msgBox.exec();
			newGraph->deleteGraph();
			return false;
		}
		Narrator * n=getNarrator(name);
		assert (n!=NULL);
		newGraph->chain.m_chain.prepend(n);
	}
	if (sel==NULL) {
		chain.m_chain.clear();
		return true;
	} else if (newGraph->isRepresentativeOf(sel->getNamesList())) {
		sel->getGraph()->deleteGraph();
		sel->graph=newGraph;
		return true;
	} else {
		msgBox.setText(QString("Nodes in the chain already constructed does not match those available in the names tags"));
		msgBox.exec();
		newGraph->deleteGraph();
		return false;
	}
}

QAbstractItemModel * HadithChainGraph::getTreeModel() {
	return new HadithChainItemModel(this);
}

void HadithChainGraph::displayGraph(ATMProgressIFC *prg) {
	QFile file("graph.dot");
	file.remove();
	if (!file.open(QIODevice::ReadWrite)) {
                theSarf->out<<"Error openning file\n";
		return;
	}
	QTextStream d_out(&file);
	d_out.setCodec("utf-8");
	d_out<<"digraph chain_graph {\n";
	for (int i=0;i<chain.m_chain.size();i++) {
		if (i!=0) {
			d_out<<"n"<<i<<"->n"<<i-1<<"\n";
		}
		Narrator * narr=dynamic_cast<Narrator *>(chain.m_chain[i]);
		QString s=narr->getString().replace("\n"," ");
		d_out<<"n"<<i<<"[label=\""<<s<<"\"]\n";
	}
	d_out<<"}\n";
	file.close();
	prg->displayGraph(this);
}

void HadithChainGraph::deleteGraph() {
#ifdef DELETE_NARRATORS
	for (int i=0;i<chain.m_chain.size();i++) {
		Narrator * narr=dynamic_cast<Narrator *>(chain.m_chain[i]);
		for (int j=0;j<narr->m_narrator.size();j++) {
			delete narr->m_narrator[j];
		}
		delete narr;
	}
#endif
	chain.m_chain.clear();
}

void HadithChainGraph::fillTextPointers(QString *text) {
	chain.hadith_text=text;
	for (int i=0;i<chain.m_chain.size();i++) {
		Narrator * narr=dynamic_cast<Narrator *>(chain.m_chain[i]);
		narr->hadith_text=text;
		for (int j=0;j<narr->m_narrator.size();j++) {
			narr->m_narrator[j]->hadith_text=text;
		}
	}
}

void HadithChainGraph::fillNullGraph(MainSelectionList &names, QString *text) {
	chain.m_chain.clear();
	for (int i=0;i<names.size();i++) {
		Name n(text,names[i].first,names[i].second);
		Narrator * narr=getNarrator(n);
		chain.m_chain.append(narr);
	}
}

void HadithChainGraph::addNameToGraph(Name &name) {
	QString s=name.getString();
	Narrator * n=getNarrator(name);
	for (int i=0;i<chain.m_chain.size();i++) {
		if (chain.m_chain[i]->getStart()>=name.getEnd()) {
			chain.m_chain.insert(i,n);
			return;
		}
	}
	chain.m_chain.append(n);
}

void HadithChainGraph::removeNameFromGraph(Name &name) {
	for (int i=0;i<chain.m_chain.size();i++) {
		Narrator * narr=dynamic_cast<Narrator *>(chain.m_chain[i]);
		if (equal_ignore_diacritics(narr->getString(),name.getString())) {
		#ifdef DELETE_NARRATORS
			for (int j=0;j<narr->m_narrator.size();j++) {
				delete narr->m_narrator[j];
			}
			delete narr;
		#endif
			chain.m_chain.removeAt(i);
		}
	}
}
