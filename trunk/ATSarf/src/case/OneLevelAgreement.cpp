#include "OneLevelAgreement.h"
#include "logger.h"
#include "text_handling.h"

OneLevelAgreement::OneLevelAgreement(QString * text,OneLevelAgreement::SelectionList & tagNames, OneLevelAgreement::SelectionList & outputNames)
	:AbstractTwoLevelAgreement(text,"",NULL,*(new OutputDataList)){
	outputList.clear();
	OutputData output(NULL);
	output.setMainInterval(0,text->size()-1);
	output.names.append(outputNames);
	outputList.append(output);
	tags.clear();
	OutputData tagsTemp(NULL);
	tagsTemp.setMainInterval(0,text->size()-1);
	tagsTemp.names.append(tagNames);
	tags.append(tagsTemp);

}

int OneLevelAgreement::calculateStatistics() {
	return calculateStatisticsHelper();
}

void OneLevelAgreement::displayStatistics(){
#ifdef DETAILED_DISPLAY
	displayed_error << "-------------------------\n"
					<< "Narrator Detection:\n"
					<< "\trecall=\t\t"<<nameRecall<<"\n"
					<< "\tprecision=\t\t"<<namePrecision<<"\n"
					<< "Narrator Boundary (Min-boundaries):\n"
					<< "\trecall=\t"<<underBoundaryRecall<<"\n"
					<< "\tprecision=\t"<<underBoundaryPrecision<<"\n"
					<< "Narrator Boundary (Max-boundaries):\n"
					<< "\trecall=\t"<<boundaryRecall<<"\n"
					<< "\tprecision=\t"<<boundaryPrecision<<"\n"
					<< "-------------------------\n";
#endif
}

int OneLevelAgreement::equalNames(QString * , int start1, int end1, int start2, int end2) {
	return overLaps(start1,end1,start2,end2);
}

OneLevelAgreement::~OneLevelAgreement() {
	delete &outputList;
}
