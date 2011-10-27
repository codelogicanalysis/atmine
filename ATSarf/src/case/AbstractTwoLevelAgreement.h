#ifndef ABSTRACTTWOLEVELAGREEMENT_H
#define ABSTRACTTWOLEVELAGREEMENT_H


#include <QString>
#include <QFile>
#include <QSet>
#include <QList>
#include "twoLevelTaggerSelection.h"
#include "abstractTwoLevelAnnotator.h"



#define DETAILED_DISPLAY
class ChainNarratorNode;
class NarratorNodeIfc;

class AbstractTwoLevelAgreement {
public:
	typedef TwoLevelSelection OutputData;
	typedef AbstractTwoLevelAnnotator::SelectionList OutputDataList;
	typedef TwoLevelSelection::MainSelection Selection;
	typedef TwoLevelSelection::MainSelectionList SelectionList;

	typedef QPair<ChainNarratorNode *,int> ChainNodeIntPair;
	typedef QPair<NarratorNodeIfc *,int> NodeIntPair;
	typedef QMap<ChainNarratorNode *, ChainNodeIntPair > ChainNodeCorrespondanceMap;
	typedef QMap<NarratorNodeIfc *, NodeIntPair > NodeCorrespondanceMap;

protected:
	QString * text;
	QString fileName;
	AbstractGraph * generatedGraph;
	OutputDataList & outputList;
	OutputDataList tags;
	AbstractGraph * annotatedGraph;
private:
	QList<int> common_i,common_j;
	QVector<double> nameRecallList, namePrecisionList,
					underNameRecallList, underNamePrecisionList,
					boundaryRecallList,boundaryPrecisionList,
					underBoundaryRecallList,underBoundaryPrecisionList;
	QList<int> currentTagLists,currentOutputLists;
	int numNames,underNumNames, numWords,underNumWords;

public: //statistics:
	int commonCount,allCommonCount;
	int generatedTagsCount,correctTagsCount;
	double segmentationRecall,
		   segmentationPrecision,
		   underSegmentationRatio,
		   nameRecall,
		   namePrecision,
		   boundaryRecall,
		   boundaryPrecision,
		   underNameRecall,
		   underNamePrecision,
		   underBoundaryRecall,
		   underBoundaryPrecision;
protected:

	int commonNames(const SelectionList & list1, const SelectionList & list2, QSet<int> & visitedTags, int & allCommon);
	void overLapMainFinished(int tagNum,int outputNum,const SelectionList & tagNames, const SelectionList & outputNames,int &numNames);
	void overLapNamesFinished(const SelectionList & tagNames, const SelectionList & outputNames,int &numWords);
public:
	AbstractTwoLevelAgreement(QString * text,QString fileName,AbstractGraph * generatedGraph, OutputDataList & generatedTags);
	int calculateStatisticsOrAnotate();
	bool readAnnotation(QString tagFileName,OutputDataList & tags,AbstractGraph * globalGraph);
	virtual void displayStatistics()=0;
private:
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2)=0;
	virtual AbstractGraph * newGraph(bool global=false)=0;
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


};

#endif // ABSTRACTTWOLEVELAGREEMENT_H
