#include <climits>
#include <QPair>
#include <QInputDialog>
#include "narratordetector.h"
#include "graph.h"
#include "narratorHash.h"
#include "mergeLists.h"
#include "biographyGraphUtilities.h"
#include "OneLevelAgreement.h"
#include "textParsing.h"


const static QString biographyTagMax = QString("") + alef_hamza_below + ba2 + ra2 + ha2 + ya2 + meem + " " + kha2 + lam
                                       + ya2 + lam;

class NarratorDetector {
    protected:
        typedef Biography::MatchingNode MatchingNode;
        typedef QList<NarratorNodesList> DoubleNarratorNodesList;
        typedef QPair<int, int> IndexPair;

        StateData currentData;
        QString *text;
        long current_pos;

        NarratorGraph *graph;
        ATMProgressIFC *prg;

    public:
        BiographyList *biographies;
        DoubleNarratorNodesList nodes;
        QList<int> paragraphDelimiters;

    protected:

        typedef QList<Narrator *> NarratorList;
        NarratorHash hash;
        NarratorList narratorList;

        class ScoredSet {
            public:
                QSet<int> setBioNarrIndicies;
                QSet<NarratorNodeIfc *> setGraphNodes;

                double score;

                ScoredSet(double score) {
                    this->score = score;
                }
        };
        typedef QList<ScoredSet> NodeGroups;
        class NodePositionMap {
            private:
                typedef QMap<NarratorNodeIfc *, int> Map;
                int max;
                Map map;
            public:
                NodePositionMap() {
                    max = 0;
                }
                int getIndex(NarratorNodeIfc *node) {
                    Map::const_iterator i = map.find(node);

                    if (i != map.end()) {
                        return i.value();
                    } else {
                        map[node] = max;
                        max++;
                        return max - 1;
                    }
                }
        };
        typedef QList<int> IndexList;
        typedef QList<IndexList *> IndexLists;

        void getSortedIndexList(NarratorNodeIfc *node, IndexList &list) {
            assert(!node->isGroupNode());
            IndexLists lists;
            FindIndiciesAction action(node, lists);

            if (node->isGraphNode()) {
                for (int i = 0; i < node->size(); i++) {
                    hash.performActionToAllCorrespondingNodes(&(*node)[i], action);
                }
            } else {
                ChainNarratorNode &c_node = dynamic_cast<ChainNarratorNode &>(*node);
                hash.performActionToAllCorrespondingNodes(&c_node, action);
            }

#ifndef ASSUMESORTED

            for (int i = 0; i < lists.size(); i++) {
                qSort(*lists[i]);
            }

#endif

            if (list.size() > 0) {
                IndexList *old = new IndexList;
                old->append(list);
                lists.append(old);
                list.clear();
            }

            MergeLists<int> merge(lists, list);
            merge.mergeLists();

            for (int i = 1; i < list.size(); i++) {
                assert(list[i - 1] <= list[i]);
            }
        }
        class GetNeighborIndexListsVisitor: public NodeVisitor {
            private:
                NarratorDetector &detector;
                NarratorNodeIfc *center;
                IndexLists &indexlists;
            public:
                GetNeighborIndexListsVisitor(NarratorDetector &aDetector, NarratorNodeIfc *center, IndexLists &lists)
                    : detector(aDetector), indexlists(lists) {
                    this->center = center;
                }
                virtual void initialize() {}
                virtual bool visit(NarratorNodeIfc &n) {
                    if (&n != center) {
                        indexlists.append(new IndexList);
                        detector.getSortedIndexList(&n, *indexlists.last());
                    }

                    return true;
                }
                virtual void visit(NarratorNodeIfc &, NarratorNodeIfc &, int) {}
                virtual void finish() {}
                virtual void detectedCycle(NarratorNodeIfc &) {}
                virtual void finishVisit(NarratorNodeIfc &) {}
        };
        class FindIndiciesAction: public NarratorHash::BiographyAction {
            private:
                NarratorNodeIfc *node;
                NodePositionMap indexMap;
                IndexLists &lists;
            public:
                FindIndiciesAction(NarratorNodeIfc *node, IndexLists &indexLists): lists(indexLists) {
                    this->node = node;
                }
                void action(const QString &, GroupNode *gnode, int index, double /*similarity*/) {
                    NarratorNodeIfc *n = &gnode->getCorrespondingNarratorNode();
                    assert(n != NULL);

                    if (n == this->node) {
                        int listIndex = indexMap.getIndex(gnode);

                        if (listIndex >= lists.size()) {
                            assert(listIndex == lists.size());
                            IndexList *PositionList = new IndexList;
                            lists.append(PositionList);
                        }

#ifdef ASSUMESORTED

                        if (lists[listIndex]->size() > 0) {
                            assert(index > lists[listIndex]->last());
                        }

#endif
                        lists[listIndex]->append(index);
                    }
                }
        };
        bool updateGroups(ScoredSet &set, NodeGroups &groups) {  //return true if changes occured
#define MAX_SIZE 3
            if (set.score >= hadithParameters.bio_threshold) {
                bool add = false;

                for (int i = 0; i < groups.size(); i++) {
                    if (groups[i].score < set.score) {
                        add = true;
                    } else if (add) {
                        groups.insert(i, set);
                        break;
                    }
                }

                if (groups.size() > MAX_SIZE) {
                    assert(add);
                    assert(groups.size() == MAX_SIZE + 1);
                    groups.removeLast();
                } else if (!add && groups.size() < MAX_SIZE) {
                    groups.append(set);
                    return true;
                }

                return add;
            }

            return false;
        }
        void addNarrators(Biography *biography) {
            Biography::NarratorNodeGroups &realNodes = biography->nodeGroups;
            int listCount = realNodes.size();

            for (int i = 0; i < listCount; i++) {
                int size = realNodes[i].size();

                if (size > 0) {
                    int index = narratorList.size();
                    narratorList.append((*biography)[i]);
                    qSort(realNodes[i]);

                    for (int j = 0; j < size; j++) {
                        Biography::MatchingNode m_node = realNodes[i][j];
                        assert(m_node.node->isGroupNode());
                        GroupNode *node = dynamic_cast<GroupNode *>(m_node.node);
                        hash.addNode(node, index);
                    }
                }
            }
        }
        inline bool areNear(int index1, int index2) {
            int diff1 = narratorList[index1]->getEnd() - narratorList[index2]->getStart(),
                diff2 = narratorList[index1]->getStart() - narratorList[index2]->getEnd();
            return absVal(diff1) < hadithParameters.bio_nrc_max ||
                   absVal(diff2) < hadithParameters.bio_nrc_max;
        }
        inline IndexPair getBoundingParagraph(int start , int end) {
            IndexPair p;
            int &p_start = p.first, & p_end = p.second;
            IndexList::const_iterator itr = qLowerBound(paragraphDelimiters, start);

            if (paragraphDelimiters.size() == 0) {
                p_start = 0;
                p_end = text->size() - 1;
            } else if ((*itr) > start) {
                if (itr == paragraphDelimiters.constBegin()) {
                    p_start = 0;
                } else {
                    p_start = *(itr - 1);
                }

                p_end = (*itr);
            } else {
                p_start = *itr;

                if (itr != paragraphDelimiters.constEnd()) {
                    itr++;
                }

                if (itr != paragraphDelimiters.constEnd()) {
                    p_end = *(itr);
                } else {
                    p_end = text->size() - 1;
                }
            }

            p_end = getLastLetter_IN_previousWord(text, p_end);

            while (isNumber(text->at(p_end))) {
                p_end--;
            }

            p_end = getLastLetter_IN_previousWord(text, p_end);

            if (end > p_end) {
                qDebug() << text->mid(start, end - start + 1);
            }

            return p;
        }
        inline IndexPair getBoundingParagraph(int index) {
            int start = narratorList[index]->getStart();
            int end = narratorList[index]->getEnd();
            return getBoundingParagraph(start, end);
        }
        inline bool isInsideBound(IndexPair &boundary, int index) {
            int start = narratorList[index]->getStart();
            int end = narratorList[index]->getEnd();
            return (boundary.first <= start && boundary.second >= end);
        }

        void findChosenBiography(NarratorNodesList &nodesList, NodeGroups &topSets) {
            topSets.clear();
            IndexList centerIndicies;
            IndexLists indexLists;

            for (int i = 0; i < nodesList.size(); i++) {
                NarratorNodeIfc *node = &(nodesList[i]->getCorrespondingNarratorNode());

                if (i == 0) {
                    prg->setCurrentAction("Checking Neighborhood for '" + node->CanonicalName() + "'");
                    //prg->report(0);
                }

                getSortedIndexList(node, centerIndicies);
                GetNeighborIndexListsVisitor visitor(*this, node, indexLists);
                GraphVisitorController cont(&visitor, NULL);
                node->BFS_traverse(cont, 1, 1);
                node->BFS_traverse(cont, 1, -1);
            }

            for (int i = 0; i < centerIndicies.size(); i++) {
                int centerIndex = centerIndicies[i];
                ScoredSet set(0);
                set.setBioNarrIndicies.insert(centerIndex);

                for (int j = 0; j < indexLists.size(); j++) {
                    IndexList &neighboringIndicies = *indexLists[j];
                    IndexList::const_iterator itr = qLowerBound(neighboringIndicies, centerIndex);

                    for (int i = 0; i < 2; i++) {
                        if (itr != neighboringIndicies.end()) {
                            int neighborIndex = *itr;
                            bool condition = areNear(centerIndex, neighborIndex);

                            if (condition) {
                                if (!set.setBioNarrIndicies.contains(neighborIndex)) {
                                    set.score++;
                                    set.setBioNarrIndicies.insert(neighborIndex);
                                    break;
                                }
                            }
                        }

                        if (itr != neighboringIndicies.begin()) {
                            itr--;
                        }
                    }
                }

                updateGroups(set, topSets);
                //prg->report((double)i/centerIndicies.size()*100+0.5);
            }

            for (int i = 0; i < indexLists.size(); i++) {
                delete indexLists[i];
            }

            //prg->report(100);
        }
        void findChosenBiography(NarratorNodeIfc *node, NodeGroups &topSets) {
            NarratorNodesList nodes;
            nodes.append(node);
            findChosenBiography(nodes, topSets);
        }

        int getNextRealNodeId(int id) {
            assert(id >= 0);

            while (id < graph->all_nodes.size()) {
                if (!graph->all_nodes[id]->isActualNode()) {
                    id++;
                } else {
                    return id;
                }
            }

            return -1;
        }
        virtual void modifyNodes() {
            if (nodes.size() == 0) {
                graph->printAllNodesList();
                bool ok;
                IndexList l;

                do {
                    int i = QInputDialog::getInt(NULL, QString("Find Biography for:"),
                                                 QString("Id of Node "), 0, 0, graph->all_nodes.size() - 1, 1, &ok);

                    if (ok) {
                        l << i;
                    }
                } while (ok);

                for (int i = 0; i < l.size(); i++) {
                    int id = getNextRealNodeId(l[i]);

                    if (id >= 0) {
                        NarratorNodesList nodeList;
                        nodeList.append(graph->getNode(id)); //TODO: add nodes equivalent to it also
                        nodes.append(nodeList);
                        qDebug() << graph->all_nodes[id]->toString();
                    }
                }
            }

            assert(nodes.size() > 0);
        }
        virtual bool checkBiography(NodeGroups & /*list*/) {
            return true;
        }
        virtual void additionalCheck(StateInfo &s) {
            bool dash = s.currentPunctuationInfo.dash;
            long finish;
            bool isNum = isNumber(text, s.startPos, finish);

            if (isNum && dash && s.endPos == finish) {
                paragraphDelimiters.append(s.nextPos);
            }
        }

        bool tagNarrator(const Narrator *n, bool /*isReal=true*/) {
            if (n->m_narrator.size() == 0) {
                return false;
            }

            prg->tag(n->getStart(), n->getLength(), Qt::darkYellow, false);

            for (int i = 0; i < n->m_narrator.size(); i++) {
                NarratorPrim *nar_struct = n->m_narrator[i];

                if (nar_struct->isNamePrim()) {
                    if (((NamePrim *)nar_struct)->learnedName) {
                        prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::blue, true);
                        //error<<nar_struct->getString()<<"\n";
                    } else {
                        prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::white, true);
                    }
                } else if (((NameConnectorPrim *)nar_struct)->isFamilyConnector()) {
                    prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::darkRed, true);
                } else if (((NameConnectorPrim *)nar_struct)->isPossessive()) {
                    prg->tag(nar_struct->getStart(), nar_struct->getLength(), Qt::darkMagenta, true);
                }
            }

            return true;
        }
    public:
        NarratorDetector(NarratorGraph *graph)
            : hash(graph) {
            this->graph = graph;
            biographies = NULL;
        }
        NarratorDetector()
            : hash(NULL) {
            biographies = NULL;
        }
        int lookup(QString input_str, ATMProgressIFC *prg)  {
            this->prg = prg;
            QFile chainOutput(chainDataStreamFileName);
            chainOutput.remove();

            if (!chainOutput.open(QIODevice::ReadWrite)) {
                return 1;
            }

            QFile input(input_str);

            if (!input.open(QIODevice::ReadOnly)) {
                theSarf->out << "File not found\n";
                return 1;
            }

            QTextStream file(&input);
            file.setCodec("utf-8");
            text = new QString(file.readAll());

            if (text == NULL) {
                theSarf->out << "file error:" << input.errorString() << "\n";
                return 1;
            }

            if (text->isEmpty()) { //ignore empty files
                theSarf->out << "empty file\n";
                return 0;
            }

            long text_size = text->size();
            currentData.initialize();
            biographies = new BiographyList;
            biographies->clear();
            HadithData *currentBiography = new HadithData(text, false, graph, input_str);
            currentBiography->initialize(text);
            currentBiography->learningEvaluator.resetLearnedNames();
            //display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
            StateInfo stateInfo;
            stateInfo.resetCurrentWordInfo();
            stateInfo.currentState = TEXT_S;
            stateInfo.nextState = TEXT_S;
            stateInfo.lastEndPos = 0;
            stateInfo.startPos = 0;
            stateInfo.processedStructure = INITIALIZE;
            stateInfo.previousPunctuationInfo.fullstop = true;

            while (stateInfo.startPos < text->length() && isDelimiter(text->at(stateInfo.startPos))) {
                stateInfo.startPos++;
            }

            prg->setCurrentAction("Parsing Biography");

            for (; stateInfo.startPos < text_size;) {
                if ((proceedInStateMachine(stateInfo, currentBiography, currentData) == false) ||
                    (stateInfo.nextPos >= text_size - 1)) {
                    addNarrators(currentBiography->biography);
                }

                additionalCheck(stateInfo);
                stateInfo.currentState = stateInfo.nextState;
                stateInfo.startPos = stateInfo.nextPos;
                stateInfo.lastEndPos = stateInfo.endPos;
                stateInfo.previousPunctuationInfo = stateInfo.currentPunctuationInfo;

                if (stateInfo.number) {
                    stateInfo.previousPunctuationInfo.fullstop = true;
                    stateInfo.previousPunctuationInfo.has_punctuation = true;
                }

                prg->report((double)stateInfo.startPos / text_size * 100 + 0.5);

                if (stateInfo.startPos == text_size - 1) {
                    break;
                }
            }

            prg->report(100);
            currentBiography->learningEvaluator.displayNameLearningStatistics();
            int tester_Counter = 1;
            biographies = new BiographyList;
            biographies->clear();
            modifyNodes();
            prg->startTaggingText(*text);
            int nodesSize = nodes.size();

            for (int i = 0; i < nodesSize; i++) {
                NarratorNodesList &nodeList = nodes[i];

                if (nodes.size() == 0) {
                    continue;
                }

                NodeGroups topSets;
                findChosenBiography(nodeList, topSets);

                if (checkBiography(topSets)) { //checkBiography also updates nodes list
                    for (int k = 0; k < topSets.size(); k++) {
                        Biography *bio = new Biography(graph, text, INT_MAX, 0);
                        biographies->append(bio);
                        QSet<int>::iterator itr = topSets[k].setBioNarrIndicies.begin();
                        int j = 0;

                        for (; itr != topSets[k].setBioNarrIndicies.end(); itr++) {
                            Narrator *n = narratorList[*itr];
                            bio->addRealNarrator(n);//addNarrator(n);
                        }

                        int start = bio->getStart();
                        int end = bio->getEnd();
                        prg->tag(start, end - start + 1, Qt::darkGray, false);
                        itr = topSets[k].setBioNarrIndicies.begin();

                        for (; itr != topSets[k].setBioNarrIndicies.end(); itr++) {
                            Narrator *n = narratorList[*itr];
                            j++;
                            bool isReal = true;

                            if (!tagNarrator(n, isReal)) {
                                theSarf->out << "found a problem an empty narrator in (" << tester_Counter << "," << j << ")\n";
                            }
                        }

                        tester_Counter++;
                    }
                }

                prg->setCurrentAction("Looking up Biographies");
                prg->report((double)i / nodesSize * 100 + 0.5);
            }

            prg->report(100);
            prg->setCurrentAction("Complete");
            prg->finishTaggingText();

            if (currentBiography != NULL) {
                delete currentBiography;
            }

            return 0;
        }

        void freeMemory() { //called if we no longer need stuctures of this class
            for (int i = 0; i < biographies->size(); i++) {
                delete(*biographies)[i];
            }

            delete text;

            for (int i = 0; i < narratorList.size(); i++) {
                delete narratorList[i];
            }
        }
};

class BiographySegmenter: protected NarratorDetector {
    private:
        class Region {
            public:
                int start, end;
                Region(int start, int end) {
                    this->start = start;
                    this->end = end;
                }
                bool operator<(const Region &rhs) const  { //check if suitable
                    //return (start<rhs.start);
                    return !overLaps(start, end, rhs.start, rhs.end) && before(start, end, rhs.start, rhs.end);
                }
                bool operator==(const Region &rhs) const {
                    //return start==rhs.start;
                    return overLaps(start, end, rhs.start, rhs.end);
                }
        };
        typedef QList<Region> RegionList;
        typedef QList<int> PositionList;
    private:
        class LocateInGraphAction: public NarratorHash::FoundAction {
            private:
                double similarity;
                NarratorNodesList &list;
            public:
                LocateInGraphAction(NarratorNodesList &narratorList): list(narratorList) {
                    similarity = 0;
                }
                virtual void action(const QString &, GroupNode *node, double similarity) {
                    if (similarity > this->similarity) {
                        this->similarity = similarity;
                        list.clear();
                        list.append(node);
                    }
                }
                QString getGroupNodeKey() {
                    if (list.size() > 0) {
                        GroupNode *node = dynamic_cast<GroupNode *>(list[0]);
                        return node->getKey();
                    } else {
                        return "<ERROR>";
                    }
                }
        };

    private:
        RegionList biographyRegions;
        PositionList paragraphDelimiters;
    public:
        using NarratorDetector::freeMemory;
        using NarratorDetector::biographies;
    public:
        BiographySegmenter(NarratorGraph *graph): NarratorDetector(graph) {}
        virtual void modifyNodes() {
            QSet<QString> narratorKeys;
            nodes.clear();

            for (int i = 0; i < narratorList.size(); i++) {
                Narrator *narr = narratorList[i];
                QString narrKey = narr->getKey();

                if (NarratorHash::isFirstNameAmbiguous(narrKey)) {
                    continue;
                }

                if (narrKey > biographyTagMax) {
                    continue;
                }

                if (narratorKeys.contains(narrKey)) {
                    continue;
                }

                NarratorNodesList nodeList;
                LocateInGraphAction action(nodeList);
                graph->performActionToAllCorrespondingNodes(narr, action);
                nodes.append(
                    nodeList); //even if not found, emptyList will be handled in lookup body, but needed to keep correspondence btw indicies of narratorList and nodes list

                if (nodeList.size() >= 0) {
                    QString key = action.getGroupNodeKey();
                    narratorKeys.insert(key);
                }
            }
        }
        virtual bool checkBiography(NodeGroups &list) {
            int start = INT_MAX, end = 0;

            if (list.size() == 0) {
                return false;
            }

            if (list.size() > 1) {
                for (int i = list.size() - 1; i > 0; i--) {
                    list.removeAt(i);
                }
            }

            //1-deduce start and end from list
            QSet<int> &bioNarrators = list[0].setBioNarrIndicies;
            QSet<int>::iterator i = bioNarrators.begin();

            for (; i != bioNarrators.end(); i++) {
                int index = *i;
                Narrator *narr = narratorList[index];
                int curr_start = narr->getStart();
                int curr_end = narr->getEnd();

                if (curr_start < start) {
                    start = curr_start;
                }

                if (curr_end > end) {
                    end = curr_end;
                }
            }

            //2-do necessary checks and add to List of biographyRegions if appropriate
            Region currRegion(start, end);
            RegionList::iterator itr = qLowerBound(biographyRegions.begin(), biographyRegions.end(), currRegion);
            //RegionList::iterator uitr=qUpperBound(biographyRegions.begin(),biographyRegions.end(),currRegion);
            bool c = (biographyRegions.size() > 0 && /*itr!=biographyRegions.begin() &&*/ itr != biographyRegions.end());

            if (c) {
                c = ((*itr) == currRegion);    //== means overLaps
            }

            if (c) {
                return false;
            }

            biographyRegions.insert(itr, currRegion);
            return true;
        }
        int segment(QString input_str, ATMProgressIFC *prg) {
            int i = lookup(input_str, prg);

            if (i >= 0) {
                i = calculateStatistics(input_str);
            }

            return i;
        }
        int calculateStatistics(QString filename) {
            OneLevelAgreement::SelectionList tags, generatedTags;

            for (int i = 0; i < biographies->size(); i++) {
                int start = biographies->at(i)->getStart();
                int end = biographies->at(i)->getEnd();
                OneLevelAgreement::Selection s(start, end);
                generatedTags.append(s);
            }

            qSort(generatedTags);
            QFile file(QString("%1.tags").arg(filename).toStdString().data());

            if (file.open(QIODevice::ReadOnly)) {
                QDataStream out(&file);   // we will serialize the data into the file
                out >> tags;
                file.close();
            } else {
                _error << "Annotation File does not exist\n";

                if (file.open(QIODevice::WriteOnly)) {
                    QDataStream out(&file);   // we will serialize the data into the file
                    out << generatedTags;
                    file.close();
                    _error << "Annotation File has been written from current detected expressions, Correct it before use.\n";
                }

                return -1;
            }

            OneLevelAgreement o(text, tags, generatedTags);
            o.calculateStatistics();
            o.displayStatistics("Biography");
            return 0;
        }
};

int biographyHelper(QString input_str, ATMProgressIFC *prg) {
    input_str = input_str.split("\n")[0];
    NarratorDetector s;
    s.lookup(input_str, prg);
    s.freeMemory();
    return 0;
}

BiographyList *getBiographies(QString input_str, NarratorGraph *graph, ATMProgressIFC *prg, int /*nodeID*/) {
    input_str = input_str.split("\n")[0];
    BiographySegmenter s(graph);
    s.segment(input_str, prg);
    return s.biographies;
}

