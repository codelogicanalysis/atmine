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
	virtual int equalNames(QString * text, int start1, int end1, int start2, int end2)=0;
protected:
	bool readAnnotations();
	int calculateStatisticsHelper();
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

template <class T>
int commonNames(QString * text, const AbstractTwoLevelAgreement::SelectionList & list1, const AbstractTwoLevelAgreement::SelectionList & list2, QSet<int> & visitedTags, int & allCommon, T & t) {
	QSet<int> visitedTags2;
	int common=0;
	allCommon=0;
	for (int i=0;i<list1.size();i++) {
		bool found=false, allCommonFound=false;
		int start1=list1[i].first,
			end1=list1[i].second;
		for (int j=0;j<list2.size();j++) {
			int start2=list2[j].first,
				end2=list2[j].second;
			if (t.equalNames(text,start1,end1,start2,end2) ) {
				if (!allCommonFound) {
					allCommonFound=true;
					allCommon++;
				}
				if (!visitedTags2.contains(j) && !visitedTags.contains(i)) {
					found=true;
					visitedTags.insert(i);
					visitedTags2.insert(j);
					break;
				}
			}
		}
		if (found)
			common++;
	}
	return common;
}

#endif // ABSTRACTTWOLEVELAGREEMENT_H
