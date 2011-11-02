#ifndef ONELEVELAGREEMENT_H
#define ONELEVELAGREEMENT_H

#include "AbstractTwoLevelAgreement.h"

class OneLevelAgreement: protected AbstractTwoLevelAgreement
{
private:
	virtual AbstractGraph * newGraph(bool ){return NULL;}
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2);

public:
	OneLevelAgreement(QString * text,SelectionList & tagNames, SelectionList & outputNames);
	int calculateStatistics();
	virtual void displayStatistics();
	~OneLevelAgreement();
};

#endif // ONELEVELAGREEMENT_H
