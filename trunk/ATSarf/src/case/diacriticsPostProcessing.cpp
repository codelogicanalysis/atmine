#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <assert.h>
#include "logger.h"
#include "diacriticsPostProcessing.h"
#include "combinations.h"

const int OneDiacriticEvaluation::vocIndex=0;
const int OneDiacriticEvaluation::sizeIndex=1;
const int OneDiacriticEvaluation::diacriticCountIndex=2;
const int OneDiacriticEvaluation::diacriticPosIndex=3;
const int OneDiacriticEvaluation::diacriticIndex=4;
const int OneDiacriticEvaluation::morphemeIndex=5;
const int OneDiacriticEvaluation::morphemeActualPosIndex=6;
const int OneDiacriticEvaluation::morphemeRelPosIndex=7;
const int OneDiacriticEvaluation::letterIndex=8;
const int OneDiacriticEvaluation::longVowelIndex=9;
const int OneDiacriticEvaluation::shamsiIndex=10;
const int OneDiacriticEvaluation::ambiguityIndexStart=12;
const QPair<int,int> OneDiacriticEvaluation::uniqueFeatureRange(0,0);
const QPair<int,int> OneDiacriticEvaluation::morphemeFeatureRange(5,7);

QString DiacriticsPostProcessing::preProcessFeature(int ,QString feature) {
	return feature;
}

QString DiacriticsPostProcessing::displayFeature(int index, QString feature) {
	QString featureName=getFeatureName(index);
	featureName+="="+feature;
	return featureName;
}

QString DiacriticsPostProcessing::getFeatureName(int index) {
	return QString("%1").arg(index);
}


QString DiacriticsPostProcessing::getKey(QStringList list, IndexList keyConstituents) {
	QString key="";
	for (int i=0;i<keyConstituents.size();i++) {
		int index=keyConstituents[i];
		QString featureVal=list[index];
		QString processedVal=preProcessFeature(index,featureVal);
		if (!key.isEmpty())
			key+="-";
		key+=QString("%1:").arg(index);
		key+=processedVal;
	}
	return key;
}

void DiacriticsPostProcessing::addToHash(StatTable & stats, QString feature, double amb, double weight){
	StatTable::iterator itr=stats.find(feature);
	if (itr==stats.end()) {
		Average a(amb,weight);
		stats.insert(feature,a);
	} else {
		Average & av=*itr;
		av.add(amb,weight);
	}
}

void DiacriticsPostProcessing::restartLumpedEntries(QString newUnique) {
	if (!lastUnique.isEmpty() || (testedFeaturesList.size()==0 && !newUnique.isEmpty())) {
		for (int i=0;i<lastPairs.size();i++) {
			int num_per_unique=0;
			for (StatTable::iterator pair_itr=lastPairs[i].begin();pair_itr!=lastPairs[i].end();pair_itr++) {
				Average & lastAverage=pair_itr.value();
				num_per_unique+=lastAverage.getDenominator();
			}
			double one_weight=1.0/num_per_unique;
			for (StatTable::iterator pair_itr=lastPairs[i].begin();pair_itr!=lastPairs[i].end();pair_itr++) {
				Average & lastAverage=pair_itr.value();
				double weight=one_weight*lastAverage.getDenominator();
				double currAmb=lastAverage.evaluate();
				QString lastFeature=pair_itr.key();
				addToHash(stats,lastFeature,currAmb,weight);
			}
		}
		totalLines++;
	}
	lastUnique=(testedFeaturesList.size()==0?"":newUnique); //if no features keep empty, we are providing a non-empty argument just to get into last if
	for (int i=0;i<lastPairs.size();i++) {
		lastPairs[i].clear();
	}
}

void DiacriticsPostProcessing::processLine(QStringList list) {
	QString uniqueKey=getKey(list,uniqueFeatures);
	double currAmb=list[ambiguityIndex].toDouble();
	if (uniqueKey!=lastUnique) { //will be lumped with old
		restartLumpedEntries(uniqueKey);
	}
	for (int i=0;i<testedFeaturesList.size();i++) {
		IndexList & featureIndicies=testedFeaturesList[i];
		QString featureKey=getKey(list,featureIndicies);
		StatTable & lastTable=lastPairs[i];
		addToHash(lastTable,featureKey,currAmb);
	}
}

void DiacriticsPostProcessing::clearStats() {
	stats.clear();
	totalLines=0;
	assert(lastUnique==""); //should be redudant since when called must be already empty but jsut in case
	for (int i=0;i<testedFeaturesList.size();i++) {
		lastPairs.append(StatTable());
	}
	restartLumpedEntries();
}

void DiacriticsPostProcessing::displayStats() {
	restartLumpedEntries(testedFeaturesList.size()==0?"<dummy>":"");
	for (StatTable::iterator itr=stats.begin();itr!=stats.end();itr++) {
		Average & av=*itr;
		QString key=itr.key();
		QStringList features=key.split('-');
		QString modifiedKey;
		for (int i=0;i<features.size();i++) {
			QStringList indexFeature=features[i].split(':');
			assert(indexFeature.size()==2);
			int index=indexFeature[0].toInt();
			QString feature=indexFeature[1];
			QString modifiedFeature=displayFeature(index,feature);
			if (!modifiedKey.isEmpty())
				modifiedKey+=", ";
			modifiedKey+=modifiedFeature;
		}
		out<<modifiedKey<<"\t("<<av.fractionOf(totalLines)<<"):\t"<<av.toString()<<'\n';
	}
	out<<"Total unique entries:\t"<<totalLines<<"\n";
}

void DiacriticsPostProcessing::checkFeatureConsistency() {
	assert(uniqueFeatures.size()>0);
#if 0
	assert(testedFeaturesList.size()>0);
	for (int i=0;i<testedFeatures.size();i++) {
		assert(uniqueFeatures.contains(testedFeatures[i]));
	}
#endif
}

void DiacriticsPostProcessing::operator()(QString fileName, ATMProgressIFC * prg) {
	checkFeatureConsistency();
	clearStats();
	QFile input(fileName);
	if (!input.open(QIODevice::ReadOnly)) {
		out << "File not found\n";
		return;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");
	long size=input.size();
	long pos=0;
	while (!file.atEnd()) {
		QString line=file.readLine(0);
		QStringList entries=line.split('\t',QString::SkipEmptyParts);
		if (entries.size()<=ambiguityIndex){
			qDebug()<<line;
			continue;
			assert (entries.size()>ambiguityIndex);
		}
		processLine(entries);
		pos+=line.toStdString().size()+1;//to account for '\n'
		prg->report((((double)pos)/size)*100+0.5);
	}
	displayStats();
	input.close();
}

QString OneDiacriticEvaluation::displayFeature(int index, QString feature) {
	switch(index) {
		case diacriticIndex: {
			int dia=feature.toInt();
			Diacritic d=(Diacritic)dia;
			QChar c=interpret_diacritic(d);
			return QString().append(c);
		} case morphemeIndex: {
			int mor=feature.toInt();
			return Morpheme::interpret((MorphemeType)mor);
		} case morphemeRelPosIndex: {
			static QString s[3]={"Begginning", "Middle", "End"};
			int pos=feature.toInt();
			return s[pos];
		} case longVowelIndex: {
			return (feature=="1")?"Vowel":"Consonant";
		} case shamsiIndex: {
			return (feature=="1"?"Shamsi":"Qamari");
		} default:
			return DiacriticsPostProcessing::displayFeature(index,feature);
	}
}

QString OneDiacriticEvaluation::getFeatureName(int index) {
	switch(index) {
	case vocIndex:
		return "Voc";
	case sizeIndex:
		return "Size";
	case diacriticCountIndex:
		return "DiaCount";
	case diacriticPosIndex:
		return "DiaPos";
	case diacriticIndex:
		return "Dia";
	case letterIndex:
		return "Letter";
	case morphemeIndex:
		return "Morph";
	case morphemeActualPosIndex:
		return "MorphPos";
	case morphemeRelPosIndex:
		return "MorphRelPos";
	case longVowelIndex:
		return "Vowel";
	case shamsiIndex:
		return "Shamsi";
	default:
		return "---";
	}
}

OneDiacriticEvaluation::OneDiacriticEvaluation(IndexList features,Ambiguity am) {
	int a=(int)am;
	ambiguityIndex=ambiguityIndexStart+a;
	for (int i=1;i<=features.size();i++) {
		CombinationGenerator gen(i,features.size());
		for (gen.begin();!gen.isFinished();++gen) {
			IndexList l=gen.getCombinationEquivalent(features);
			testedFeaturesList.append(l);
		}
	}

	for (int i=uniqueFeatureRange.first;i<=uniqueFeatureRange.second;i++)
		uniqueFeatures.append(i);
#if 0
	bool morphfeat=false;
	for (int i=0;i<features.size();i++) {
		int feature=features[i];
		if (morphemeFeatureRange.first<=feature && feature<=morphemeFeatureRange.second) {
			morphfeat=true;
			break;
		}
	}
	if (morphfeat) {
		for (int i=morphemeFeatureRange.first;i<=morphemeFeatureRange.second;i++)
			uniqueFeatures.append(i);
	}
#endif
}

int diacriticStatistics(QString inputString, ATMProgressIFC * prg) {
	OneDiacriticEvaluation::IndexList list;
#if 0
	//list.append(OneDiacriticEvaluation::letterIndex);
	list.append(OneDiacriticEvaluation::diacriticIndex);
	list.append(OneDiacriticEvaluation::morphemeIndex);
	list.append(OneDiacriticEvaluation::morphemeRelPosIndex);
	list.append(OneDiacriticEvaluation::longVowelIndex);
	list.append(OneDiacriticEvaluation::shamsiIndex);
#endif
	OneDiacriticEvaluation e(list,/*Stem_Ambiguity*/Vocalization);
	e(inputString,prg);
	return 0;
}
