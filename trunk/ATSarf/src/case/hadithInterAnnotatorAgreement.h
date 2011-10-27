#ifndef HADITHINTERANNOTATORAGREEMENT_H
#define HADITHINTERANNOTATORAGREEMENT_H

#include "AbstractTwoLevelAgreement.h"

#define GRAPH_COMPARE

class HadithInterAnnotatorAgreement : public AbstractTwoLevelAgreement
{
#ifdef GRAPH_COMPARE
private:
	typedef QPair<ChainNarratorNode *,int> ChainNodeIntPair;
	typedef QPair<NarratorNodeIfc *,int> NodeIntPair;
	typedef QMap<ChainNarratorNode *, ChainNodeIntPair > ChainNodeCorrespondanceMap;
	typedef QMap<NarratorNodeIfc *, NodeIntPair > NodeCorrespondanceMap;
private:
	ChainNodeCorrespondanceMap chainNodesMapCO,chainNodesMapOC,nodesMap;
	ChainNarratorNode	* nodeCorrect,
						* nodeDetected;
	int countCommon;
	double mergesRecallALL,mergesPrecisionALL,mergesRecallNew,mergesPrecisionNew;
#endif
public:
	HadithInterAnnotatorAgreement(QString * text,QString fileName,AbstractGraph * generatedGraph, OutputDataList & generatedTags);
	void displayStatistics();
private:
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2);
	virtual AbstractGraph * newGraph(bool global=false);
	virtual void initializeAuxiliaryCountersLists();
	virtual void startNamesOverLap(int tagCnt, int outputCount, int tagNameCount,int outputTagCount, int commonCount);
	virtual void anotherTagOverLapPreviousOutputName(int tagCnt, int outputCount, int tagNameCount,int outputTagCount);
	virtual void anotherOutputOverLapPreviousTagName(int tagCnt, int outputCount, int tagNameCount,int outputTagCount);
	virtual void firstNameOverLap(int tagCnt, int outputCount, int tagNameCount,int outputTagCount);
	virtual void beforeMovingToNextTagName(int tagCnt, int outputCount, int tagNameCount,int outputTagCount);
	virtual void beforeMovingToNextOutputName(int tagCnt, int outputCount, int tagNameCount,int outputTagCount);
	virtual void beforeMovingToNextOutput(int tagCnt, int outputCount) ;
	virtual void beforeMovingToNextTag(int tagCnt, int outputCount);
	virtual void compareGlobalGraphs(AbstractGraph * correctGraph,AbstractGraph * generaredGraph);
	int getNumChainNodes(NarratorNodeIfc * node);
	int getCommonNodes(NarratorNodeIfc * correctNode,NarratorNodeIfc * detectedNode);
};

#endif // HADITHINTERANNOTATORAGREEMENT_H
