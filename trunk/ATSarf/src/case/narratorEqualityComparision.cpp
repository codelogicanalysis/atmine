#include <QFile>
#include <QString>
#include "ATMProgressIFC.h"
#include "editDistance.h"
#include "narrator_abstraction.h"
#include "narratorEqualityModel.h"
#include "hadithChainGraph.h"
#include "hadithCommon.h"

//#define DETAILED_DISPLAY

#define max(a,b) (a>b? a:b)

int narrator_equality_comparision(QString input_str, ATMProgressIFC *) {
	typedef NarratorEqualityModel::NarratorMap NarratorMap;
	NarratorMap map;
	if (input_str.isEmpty())
		return -1;
	QString filename=input_str.split("\n",QString::SkipEmptyParts)[0];
	QFile file(QString("%1.equal").arg(filename).toStdString().data());
	if (file.open(QIODevice::ReadOnly))	{
		QDataStream in(&file);   // we will serialize the data into the file
		in>>map;
		file.close();
	} else {
		error << "Narrator Equality Annotation File does not exist\n";
		return -1;
	}
	int countAnnotationEqual=0, countCorrectEqualeNarrator=0, countCorrectEqualOur=0,
		countAllEqualeNarrator=0, countAllEqualOur=0;
	NarratorMap::iterator itr=map.begin();
	for (;itr!=map.end();++itr) {
		QString n1=itr.key().first;
		QString n2=itr.key().second;
		bool eq=*itr;
		if (eq)
			countAnnotationEqual++;
		int editDistance=EditDistance::Compute(n1,n2);
		double max=max(n1.size(),n2.size());
		bool eNarratorEqual=(1-(double)editDistance/max>0.75);
		if (eNarratorEqual)
			countAllEqualeNarrator++;
		if (eq && eNarratorEqual)
			countCorrectEqualeNarrator++;
		Narrator * narr1=HadithChainGraph::getNarrator(n1);
		Narrator * narr2=HadithChainGraph::getNarrator(n2);
		double equalScore=equal(*narr1,*narr2);
		bool equalOur=equalScore>hadithParameters.equality_threshold;
		if (equalOur)
			countAllEqualOur++;
		if (eq && equalOur)
			countCorrectEqualOur++;
		delete narr1;
		delete narr2;
	#ifndef DETAILED_DISPLAY
		if (eq ==!(equalOur || eNarratorEqual))
	#endif
			displayed_error<<n1<<"\t"<<n2<<"\t"<<eq<<"\t"<<equalOur<<"\t"<<eNarratorEqual<<"\n";
	}
	displayed_error <<	"eNarrator:\n"
					<<  "\trecall:\t"<<countCorrectEqualeNarrator<<"/"<<countAnnotationEqual<<"=\t"
							<<(double)countCorrectEqualeNarrator/countAnnotationEqual<<"\n"
					<<  "\tprecision:\t"<<countCorrectEqualeNarrator<<"/"<<countAllEqualeNarrator<<"=\t"
							<<(double)countCorrectEqualeNarrator/countAllEqualeNarrator<<"\n"
					<<	"Our Equality Metric:\n"
					<<  "\trecall:\t"<<countCorrectEqualOur<<"/"<<countAnnotationEqual<<"=\t"
							<<(double)countCorrectEqualOur/countAnnotationEqual<<"\n"
					<<  "\tprecision:\t"<<countCorrectEqualOur<<"/"<<countAllEqualOur<<"=\t"
							<<(double)countCorrectEqualOur/countAllEqualOur<<"\n"
					<<"Comparision Statistics:\n"
					<<"\tNarrator Pairs=\t"<<map.size()<<"\n"
					<<"\tEqual Pairs=\t"<<countAnnotationEqual<<"\n";
	return 0;
}
