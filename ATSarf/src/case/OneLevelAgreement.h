#ifndef ONELEVELAGREEMENT_H
#define ONELEVELAGREEMENT_H

#include "AbstractTwoLevelAgreement.h"

class OneLevelAgreement: protected AbstractTwoLevelAgreement
{
public:
	using AbstractTwoLevelAgreement::Selection;
	using AbstractTwoLevelAgreement::SelectionList;
private:
	virtual AbstractGraph * newGraph(bool ){return NULL;}
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2);
	void initialize(QString * text,SelectionList & tagNames, SelectionList & outputNames);
public:
	OneLevelAgreement(QString * text,SelectionList & tagNames, SelectionList & outputNames);
	int calculateStatistics();
	void displayStatistics(QString structure);
	virtual void displayStatistics() {
		displayStatistics("Narrator");
	}

	~OneLevelAgreement();
};

#endif // ONELEVELAGREEMENT_H
