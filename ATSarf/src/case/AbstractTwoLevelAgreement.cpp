#include "AbstractTwoLevelAgreement.h"
#include "logger.h"
#include "text_handling.h"
#include "Math_functions.h"
#include "hadithCommon.h"

AbstractTwoLevelAgreement::AbstractTwoLevelAgreement(QString * text,QString fileName,AbstractGraph * generatedGraph, OutputDataList & generatedTags)
	:outputList(generatedTags) {
		this->text=text;
		this->fileName=fileName;
		this->generatedGraph=generatedGraph;
		tags.clear();
		annotatedGraph=NULL;
		numNames=underNumNames=numWords=underNumWords;
		commonCount=allCommonCount=generatedTagsCount=correctTagsCount=0;
		segmentationRecall=segmentationPrecision=underSegmentationRatio=
			nameRecall=namePrecision=boundaryRecall=boundaryPrecision=
			underNameRecall=underNamePrecision=underBoundaryRecall=underBoundaryPrecision=0;
}


int AbstractTwoLevelAgreement::commonNames(const SelectionList & list1, const SelectionList & list2, QSet<int> & visitedTags, int & allCommon) {
	return ::commonNames<AbstractTwoLevelAgreement>(text,list1,list2,visitedTags,allCommon,*this);
}

void AbstractTwoLevelAgreement::overLapNamesFinished(const SelectionList & tagWords, const SelectionList & outputWords,int &numWords) {
	if (tagWords.size()==0 && outputWords.size()==0 )
		return;
	bool underComputation=(&numWords==&underNumWords);
	int countCommon=commonWords(text,tagWords,outputWords);
	int countCorrect=countWords(text,tagWords);
	int countDetected=countWords(text,outputWords);
	assert (countCorrect!=0);
	assert (countDetected!=0);
	numWords+=countCorrect;
	double	recall=(double)countCommon/countCorrect * countCorrect,
			precision=(double)countCommon/countDetected *countCorrect;
	if (countDetected==0)
		precision=0;
	if (underComputation) {
		underBoundaryRecallList.append(recall);
		underBoundaryPrecisionList.append(precision);
	} else {
		boundaryRecallList.append(recall);
		boundaryPrecisionList.append(precision);
	}
}

void AbstractTwoLevelAgreement::overLapMainFinished(int i,int j,const SelectionList & tagNames, const SelectionList & outputNames,int &numNames) {
	bool underComputation=(&numNames==&underNumNames);
	assert(underComputation || (i<0 && j<0 )); //if in max-boundary, i and j have no significance
	QSet<int> visitedTags;
	int allCommonCount;
	int countCommon=commonNames(tagNames,outputNames,visitedTags,allCommonCount);
	int countCorrect=tagNames.size();
	int countDetected=outputNames.size();
	assert (countCommon<=countDetected);
	if (countCorrect>0) {
		numNames+=countCorrect;
		double	recall=(double)countCommon/countCorrect * countCorrect,
				precision=(double)countCommon/countDetected *countCorrect;
		if (countDetected==0)
			precision=0;
		if (underComputation) {
			underNameRecallList.append(recall);
			underNamePrecisionList.append(precision);
		} else {
			nameRecallList.append(recall);
			namePrecisionList.append(precision);
		}
		int k=0,h=0;
		QList<int> common_k,common_h;
		SelectionList tagWords, outputWords;
		while (k<tagNames.size() && h<outputNames.size()) {
			int start1=tagNames[k].first,end1=tagNames[k].second,
				start2=outputNames[h].first,end2=outputNames[h].second;
			if (overLaps(start1,end1,start2,end2) && start1!=end2) {
				bool foundK=common_k.contains(k), foundH=common_h.contains(h);
				if (underComputation)
					startNamesOverLap(i,j,k,h,countCommon);
				if (!foundK /*&& !foundJ*/) {//so that merged parts will not be double counted
					common_k.append(k);
				} else if (foundH) { //new correctnode that matches detected
					tagWords.append(tagNames[k]);
					if (underComputation)
						anotherTagOverLapPreviousOutputName(i,j,k,h);
				}
				if (!foundH) {//common_i and common_j now are not same size, bc recall and precision need different treatment for overlap
					common_h.append(h);
				} else if (foundK) { //new detectednode that matches correct
					outputWords.append(outputNames[h]);
					if (underComputation)
						anotherOutputOverLapPreviousTagName(i,j,k,h);
				}
				if (!foundK && !foundH) {
					tagWords.append(tagNames[k]);
					outputWords.append(outputNames[h]);
					if (underComputation)
						firstNameOverLap(i,j,k,h);
				}
				//[underboundary computations
					SelectionList singleTagWords, singleOutputWords;
					singleTagWords.append(tagNames[k]);
					singleOutputWords.append(outputNames[h]);
					overLapNamesFinished(singleTagWords,singleOutputWords,underNumWords);
				//]
				int process=0;
				if (end1<=end2 ) {
					k++;
					process++;
				}
				if (end2<=end1) {
					h++;
					process++;
				}
				if (process==2) {
					//[max-boundary computations
					overLapNamesFinished(tagWords,outputWords,numWords);
					//]
					tagWords.clear();
					outputWords.clear();
				}
			} else if (before(start1,end1,start2,end2)) {
				if (underComputation)
					beforeMovingToNextTagName(i,j,k,h);
				//[max-boundary computations
				overLapNamesFinished(tagWords,outputWords,numWords);
				//]
				tagWords.clear();
				outputWords.clear();
				k++;
			} else if (after(start1,end1,start2,end2) ) {
				if (underComputation)
					beforeMovingToNextOutputName(i,j,k,h);
				//[max-boundary computations
				overLapNamesFinished(tagWords,outputWords,numWords);
				//]
				tagWords.clear();
				outputWords.clear();
				h++;
			}
		}
	} else {
		nameRecallList.append(0);
		namePrecisionList.append(0);
	}
	if (underComputation) { //under computation only
		if (hadithParameters.detailed_statistics) {
			displayed_error	<</*text->mid(start1,end1-start1+1)*/i<<"\t"
							<</*text->mid(start2,end2-start2+1)*/j<<"\t"
							<<countCommon<<"/"<<countCorrect<<"\t"<<allCommonCount<<"/"<<countDetected<<"\n";
		}
	}
}


bool AbstractTwoLevelAgreement::readAnnotation(QString tagFileName,OutputDataList & tags,AbstractGraph * globalGraph) {
	AbstractGraph * dummyLocalGraph=newGraph(false);
	QFile file(tagFileName.toStdString().data());
	if (file.open(QIODevice::ReadOnly))	{
		QDataStream in(&file);   // we will serialize the data into the file
		tags.readFromStream(in,dummyLocalGraph); //HadithChainGraph provided but will not be in use after call just duplicated
		globalGraph->readFromStream(in);
		file.close();
		delete dummyLocalGraph;
		return true;
	}
	return false;
}

bool AbstractTwoLevelAgreement::readAnnotations() {
	annotatedGraph=newGraph(true);
	if (!readAnnotation(fileName+".tags",tags,annotatedGraph)){
		error << "Annotation File does not exist\n";
	#ifndef SUBMISSION
		QFile file(QString("%1.tags").arg(fileName).toStdString().data());
		if (file.open(QIODevice::WriteOnly)) {
			QDataStream out(&file);   // we will serialize the data into the file
			out << outputList;
			generatedGraph->writeToStream(out);;
			file.close();
			error << "Annotation File has been written from current detected expressions, Correct it before use.\n";
		}
	#endif
		return false;
	}
	return true;
}

int AbstractTwoLevelAgreement::calculateStatisticsHelper() {

	common_i.clear();
	common_j.clear();
	nameRecallList.clear();
	namePrecisionList.clear();
	boundaryRecallList.clear();
	boundaryPrecisionList.clear();
	underNameRecallList.clear();
	underNamePrecisionList.clear();
	underBoundaryRecallList.clear();
	underBoundaryPrecisionList.clear();

	qSort(tags.begin(),tags.end());
	for (int i=0;i<tags.size();i++) {
		tags[i].setText(text);
		AbstractGraph * g=tags[i].getGraph();
		if (g!=NULL)
			g->fillTextPointers(text);

	}
	if (annotatedGraph!=NULL)
		annotatedGraph->fillTextPointers(text);

	initializeAuxiliaryCountersLists();
	currentTagLists.clear();
	currentOutputLists.clear();
	numNames=0;
	underNumNames=0;
	numWords=0;
	underNumWords=0;

	int i=0,j=0;
	SelectionList tagNamesOverLap, outputNamesOverLap;
	while (i<tags.size() && j<outputList.size()) {
		int start1=tags[i].getMainStart(),end1=tags[i].getMainEnd(),
			start2=outputList[j].getMainStart(),end2=outputList[j].getMainEnd();
		if (overLaps(start1,end1,start2,end2) && start1!=end2) {
			const SelectionList & tagNames=tags[i].getNamesList(),
								& outputNames=outputList[j].getNamesList();
			bool foundI=common_i.contains(i), foundJ=common_j.contains(j);
			if (!foundI /*&& !foundJ*/) {//so that merged parts will not be double counted
				common_i.append(i);
			} else {
				currentOutputLists.append(j);
			}
			if (!foundJ) {//common_i and common_j now are not same size, bc recall and precision need different treatment for overlap
				common_j.append(j);
			} else {
				currentTagLists.append(i);
			}
			//[underboundary computations
				overLapMainFinished(i,j,tagNames,outputNames,underNumNames);
			//]
				tagNamesOverLap.append(tagNames);
				outputNamesOverLap.append(outputNames);

			int progress=0;
			if (end1<=end2 ) {
				i++;
				progress++;
			}
			if (end2<=end1) {
				j++;
				progress++;
			}
			if (progress==2) {
				//[max-boundary computations
				overLapMainFinished(-1,-1,tagNamesOverLap,outputNamesOverLap,numNames);
				//]
				tagNamesOverLap.clear();
				outputNamesOverLap.clear();
			}
		} else if (before(start1,end1,start2,end2)) {
			if (hadithParameters.detailed_statistics) {
				displayed_error	<</*text->mid(start1,end1-start1+1)*/i<<"\t"
								<<"-----\n";
			}
			//[max-boundary computations
			overLapMainFinished(-1,-1,tagNamesOverLap,outputNamesOverLap,numNames);
			//]
			tagNamesOverLap.clear();
			outputNamesOverLap.clear();
			beforeMovingToNextTag(i,j);
			i++;
		} else if (after(start1,end1,start2,end2) ) {
			if (hadithParameters.detailed_statistics) {
				displayed_error	<<"-----\t"
								<</*text->mid(start2,end2-start2+1)*/j<<"\n";
			}
			//[max-boundary computations
			overLapMainFinished(-1,-1,tagNamesOverLap,outputNamesOverLap,numNames);
			//]
			tagNamesOverLap.clear();
			outputNamesOverLap.clear();
			beforeMovingToNextOutput(i,j);
			j++;
		}
	}
	if (tagNamesOverLap.size()>0 || outputNamesOverLap.size()>0) {
		//[max-boundary computations
		overLapMainFinished(-1,-1,tagNamesOverLap,outputNamesOverLap,numNames);
		//]
		tagNamesOverLap.clear();
		outputNamesOverLap.clear();
	}
	while (i<tags.size()) {
		if (hadithParameters.detailed_statistics) {
			//int start1=tags[i].getMainStart(),end1=tags[i].getMainEnd();
			displayed_error <</*text->mid(start1,end1-start1+1)*/i<<"\t"
							<<"-----\n";
		}
		beforeMovingToNextTag(i,j);
		i++;
	}
	while (j<outputList.size()) {
		if (hadithParameters.detailed_statistics) {
			//int start2=outputList[j].getMainStart(),end2=outputList[j].getMainEnd();
			displayed_error <<"-----\t"
							<</*text->mid(start2,end2-start2+1)*/j<<"\n";
			beforeMovingToNextOutput(i,j);
		}
		j++;
	}

	commonCount=common_i.size();
	allCommonCount=common_j.size();
	correctTagsCount=tags.size();
	generatedTagsCount=outputList.size();
	segmentationRecall=(double)commonCount/tags.size();
	segmentationPrecision=(double)allCommonCount/outputList.size();
	underSegmentationRatio=(double)commonCount/allCommonCount;
	nameRecall=sum(nameRecallList)/numNames;
	namePrecision=sum(namePrecisionList)/numNames;
	boundaryRecall=sum(boundaryRecallList)/numWords;
	boundaryPrecision=sum(boundaryPrecisionList)/numWords;
	underNameRecall=sum(underNameRecallList)/underNumNames;
	underNamePrecision=sum(underNamePrecisionList)/underNumNames;
	underBoundaryRecall=sum(underBoundaryRecallList)/underNumWords;
	underBoundaryPrecision=sum(underBoundaryPrecisionList)/underNumWords;

	return 0;
}

int AbstractTwoLevelAgreement::calculateStatisticsOrAnotate() {
	if (!readAnnotations()) {
		return -1;
	}
	int ret=calculateStatisticsHelper();
	compareGlobalGraphs(annotatedGraph,generatedGraph);
	for (int i=0;i<tags.size();i++)
		tags[i].getGraph()->deleteGraph();
	return ret;
}

void AbstractTwoLevelAgreement::initializeAuxiliaryCountersLists() { }

void AbstractTwoLevelAgreement::startNamesOverLap(int , int , int ,int, int) { }

void AbstractTwoLevelAgreement::anotherTagOverLapPreviousOutputName(int , int , int ,int ) { }

void AbstractTwoLevelAgreement::anotherOutputOverLapPreviousTagName(int , int , int ,int ) { }

void AbstractTwoLevelAgreement::firstNameOverLap(int , int , int ,int ) { }

void AbstractTwoLevelAgreement::beforeMovingToNextTagName(int , int , int ,int ) { }

void AbstractTwoLevelAgreement::beforeMovingToNextOutputName(int , int , int ,int ) { }

void AbstractTwoLevelAgreement::beforeMovingToNextOutput(int , int ) { }

void AbstractTwoLevelAgreement::beforeMovingToNextTag(int , int ) { }

void AbstractTwoLevelAgreement::compareGlobalGraphs(AbstractGraph * ,AbstractGraph * ) { }

