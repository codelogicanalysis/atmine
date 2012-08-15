#ifndef BIBLEMANUALTAGGER_H
#define BIBLEMANUALTAGGER_H

#include "abstractTwoLevelAnnotator.h"

class GeneMainWindow;

class BibleTaggerDialog:public AbstractTwoLevelAnnotator{
	Q_OBJECT
public:
	BibleTaggerDialog(QString filename);
private:
	virtual void regenerateGlobalGraph();
	virtual AbstractGraph * newGraph(bool global);

public:
	virtual ~BibleTaggerDialog();

};



#endif // BIBLEMANUALTAGGER_H
