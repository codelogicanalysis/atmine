#include "twoLevelTaggerSelection.h"
#include "bibleGeneology.h"
#include "abstractAnnotator.h"

int Selection::getNameIndex(QString s) {
	for (int i=0;i<names.size();i++) {
		if (equalNames(Name(text,names[i].first,names[i].second).getString(),s))
			return i;
	}
	return -1;
}

void Selection::removeExtraNames() {
	for (int i=0;i<names.size();i++) {
		if (!AbstractAnnotator::isConsistentWithSelectionCondidtion(main.first,main.second,names[i].first,names[i].second,AbstractAnnotator::SELECTION_OUTSIDE)) {
			removeNameAt(i);
			i--;
		}
	}
}

Selection::Selection():main(-1,-1) {
	graph=NULL;
	text=NULL;
}
Selection::Selection(QString * text,int start,int end):main(start,end) {
	graph=NULL;
	this->text=text;
}
void Selection::addName(int start,int end) {
	names.append(MainSelection(start,end));
	fillNullGraph();
	if (names.size()>1) {
		Name n(text,start,end);
		addNameToGraph(n);
	}
}
void Selection::addName( const Name & name) {
	addName(name.getStart(),name.getEnd());
}

void Selection::removeNameAt(int i) {
	MainSelection s=names[i];
	names.removeAt(i);
	Name n(text,s.first,s.second/*-1*/);
	removeNameFromGraph(n);
}

void Selection::setMainInterval(int start,int end) {
	main.first=start;
	main.second=end;
}
void Selection::setGraph(AbstractGraph * graph) { //must not be used by tagger just by possibly the parser
	this->graph=graph;
}
void Selection::setText(QString *text) { //must not be used by tagger just by possibly the parser
	this->text=text;
}
void Selection::clear() {
	names.clear();
	graph=NULL;
}
int Selection::mergeNames(QString * text,const MainSelectionList & list1, const MainSelectionList & list2,MainSelectionList & mergedNames){
	mergedNames.clear();
	mergedNames.append(list2);
	for (int i=0;i<list1.size();i++) {
		bool found=false;
		for (int j=0;j<mergedNames.size();j++) {
			QString s1=Name(text,mergedNames[j].first,mergedNames[j].second).getString(),
					s2=Name(text,list1[i].first,list1[i].second).getString();
			if (equalNames(s1,s2) ) {
				found=true;
				break;
			}
		}
		if (!found) {
			mergedNames.append(list1[i]);
		}
	}
	return mergedNames.size();
}

