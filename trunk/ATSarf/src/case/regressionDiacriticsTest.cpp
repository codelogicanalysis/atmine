#include "decisionTreeRegression.h"
#include "Tree2Dot.h"
#include "diacriticsPostProcessing.h"

class RegressionTree2Dot: public Tree2Dot<RegressionNodeType, RegressionEdgeType> {
protected:
	virtual QString getExtraNodeLayout(Node<RegressionNodeType, RegressionEdgeType> * node) {
		if (node->isLeaf()) {
			return "style=filled, shape=box, fillcolor=gray";
		}
		return "";
	}
public:
	RegressionTree2Dot(QString file):Tree2Dot<RegressionNodeType, RegressionEdgeType>(file) {}
};


int regressionTest(QString input, ATMProgressIFC * prg) {
	QStringList entries=input.split('\t');
	QString fileName=entries[0];
	double threshold=(entries.size()>1)?entries[1].toDouble():0.05;
	double minInstances=(entries.size()>2)?entries[2].toDouble():3;
	DecisionTreeRegression::TerminationRule rule(minInstances,threshold);
	QList<int> list;
#if 0
	list.append(OneDiacriticEvaluation::letterIndex);
	list.append(OneDiacriticEvaluation::diacriticIndex);
	list.append(OneDiacriticEvaluation::morphemeIndex);
	list.append(OneDiacriticEvaluation::morphemeRelPosIndex);
	list.append(OneDiacriticEvaluation::longVowelIndex);
	list.append(OneDiacriticEvaluation::shamsiIndex);
	int weightColumn=-1;
	int targetColumn=OneDiacriticEvaluation::ambiguityIndexStart;

#else
	list.append(0);
	list.append(1);
	list.append(2);
	list.append(3);
	int weightColumn=-1;
	int targetColumn=5;
#endif
	DecisionTreeRegression r(fileName,list,weightColumn,targetColumn,rule);
	RegressionTree * tree=r.buildTree(prg);
	RegressionTree2Dot d("g.dot");
	d.DFS(*tree);
	return 0;
}

