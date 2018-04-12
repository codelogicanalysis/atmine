#ifndef GENEALOGYINTERANNOTATORAGREEMENT_H
#define GENEALOGYINTERANNOTATORAGREEMENT_H

#include "AbstractTwoLevelAgreement.h"

class GenealogyInterAnnotatorAgreement : public AbstractTwoLevelAgreement
{
public:
	GenealogyInterAnnotatorAgreement(QString * text,QString fileName,AbstractGraph * generatedGraph, OutputDataList & generatedTags);
	void displayStatistics();
private:
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2);
	virtual AbstractGraph * newGraph(bool global=false);
};

#endif // GENEALOGYINTERANNOTATORAGREEMENT_H
