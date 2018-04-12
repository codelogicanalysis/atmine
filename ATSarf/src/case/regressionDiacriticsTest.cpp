#include "decisionTreeRegression.h"
#include "Tree2Dot.h"
#include "TreeFilter.h"
#include "diacriticsPostProcessing.h"

class RegressionTree2Dot: public Tree2Dot<RegressionNodeType, RegressionEdgeType> {
    protected:
        virtual QString getExtraNodeLayout(Node<RegressionNodeType, RegressionEdgeType> *node) {
            if (node->isLeaf()) {
                return "style=filled, shape=box, fillcolor=gray";
            }

            return "";
        }
    public:
        RegressionTree2Dot(QString file): Tree2Dot<RegressionNodeType, RegressionEdgeType>(file) {}
};

int regressionTest(QString input, ATMProgressIFC *prg) {
    QStringList entries = input.split('\t');
    QString fileName = entries[0];
    double threshold = (entries.size() > 1) ? entries[1].toDouble() : 0.05;
    double minInstances = (entries.size() > 2) ? entries[2].toDouble() : 3;
    DecisionTreeRegression::TerminationRule rule(minInstances, threshold);
    QList<int> list;
    //list.append(OneDiacriticEvaluation::letterIndex);
    list.append(OneDiacriticEvaluation::diacriticIndex);
    list.append(OneDiacriticEvaluation::morphemeIndex);
    list.append(OneDiacriticEvaluation::morphemeRelPosIndex);
    list.append(OneDiacriticEvaluation::longVowelIndex);
    list.append(OneDiacriticEvaluation::shamsiIndex);
    list.append(12);
    int weightColumn = 14; //-1
    int targetColumn = OneDiacriticEvaluation::ambiguityIndexStart;
    //minInstances=20;
    DecisionTreeRegression r(fileName, list, weightColumn, targetColumn, rule, prg);
    RegressionTree *tree = r.buildTree(prg);
    RegressionTree2Dot d("g.dot");
    d.DFS(*tree);
    QFile file("g.tree");

    if (!file.open(QIODevice::WriteOnly)) {
        return -1;
    }

    QDataStream o(&file);
    o << *tree;
    return 0;
}

class ThresholdFilterVisitor: public TreeFilter<RegressionNodeType, RegressionEdgeType> {
    private:
        double threshold;
    protected:
        bool shouldBeRemoved(RegressionNode *node) {
            if (node->isLeaf()) {
                RegressionNodeType t = node->getValue();
                double val = t.toDouble();
                return (val > threshold);
            }

            return false;
        }
    public:
        ThresholdFilterVisitor(double thr): TreeFilter<RegressionNodeType, RegressionEdgeType>(), threshold(thr) {}
};

int regressionReload(QString input, ATMProgressIFC * /*prg*/) {
    RegressionTree *tree = new RegressionTree();
    QFile file("g.tree");

    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }

    QDataStream i(&file);
    i >> *tree;
    double threshold = input.toDouble();
    ThresholdFilterVisitor v(threshold);
    v.DFS(*tree);
    RegressionTree2Dot d("g1.dot");
    d.DFS(*tree);
    return 0;
}

