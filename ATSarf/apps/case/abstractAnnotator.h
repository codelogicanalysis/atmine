#ifndef ABSTRACTANNOTATOR_H
#define ABSTRACTANNOTATOR_H

#include <QTextBrowser>

class AbstractAnnotator {
public:
	enum SelectionMode {SELECTION_OUTSIDE,SELECTION_INSIDE,SELECTION_OUTSIDEOVERLAP};
	inline static bool isConsistentWithSelectionCondidtion(int start, int end, int tagStart,int tagEnd,SelectionMode selectionMode=SELECTION_OUTSIDE) {
		bool con1= selectionMode==SELECTION_OUTSIDE && start<=tagStart && end>=tagEnd,
			 con2= selectionMode==SELECTION_INSIDE && start>=tagStart && end<=tagEnd,
			 con3= selectionMode==SELECTION_OUTSIDEOVERLAP && ((start<=tagStart && end>=tagStart) ||
														(start<=tagEnd && end>=tagEnd));
		return con1 || con2 || con3;
	}
	int findSelection(int startIndex=0, SelectionMode selectionMode=SELECTION_OUTSIDE); //based on user text selection
	void moveSelectionToWordBoundaries() ;
	void moveSelectionToSentenceBoundaries();
private:
	virtual QTextBrowser * getTextBrowser()=0;
	virtual int getTagCount() const =0;
	virtual int getTagStart(int tagNum) const=0;
	virtual int getTagEnd(int tagNum) const=0;
};

#endif // ABSTRACTANNOTATOR_H
