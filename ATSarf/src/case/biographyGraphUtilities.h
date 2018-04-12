#ifndef BIOGRAPHYGRAPHUTILITIES_H
#define BIOGRAPHYGRAPHUTILITIES_H

#include "narratorHash.h"

class RealNarratorAction: public NarratorHash::FoundAction {
    private:
        Biography::NarratorNodeList &list;
        bool found;
    public:
        RealNarratorAction(Biography::NarratorNodeList &nodeList): list(nodeList) {}
        virtual void action(const QString &, GroupNode *node, double similarity) {
            if (similarity > hadithParameters.equality_threshold) {
                found = true;
                NarratorNodeIfc *n = node;
                int index;

                if (!Biography::MatchingNode::contains(list, n, similarity, index)) {
                    list.append(Biography::MatchingNode(n, similarity));
                } else if (index >= 0) { //i.e. found but with a smaller similarity
                    list[index] = Biography::MatchingNode(n, similarity);
                }
            }
        }
        void resetFound() {
            found = false;
        }
        bool isFound() {
            return found;
        }
};
bool isRealNarrator(NarratorGraph *graph, Narrator *n, Biography::NarratorNodeList &list);

class BiographySegmenterAlgorithm {
    public:
        virtual bool addNarrator(Narrator *n) = 0;
};

#endif
