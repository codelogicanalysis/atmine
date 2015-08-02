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
#ifdef SEGMENT_BIOGRAPHY_USING_POR
        typedef Biography::MatchingNode MatchingNode;
#endif
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

#ifdef SEGMENT_BIOGRAPHY_USING_POR
#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
#ifndef FAST_NEIGHBOR_CHECK
        class ReachableVisitor: public NodeVisitor {
                NarratorNodeIfc *target;
                bool found;
            public:
                ReachableVisitor(NarratorNodeIfc *aTarget) {
                    target = aTarget;
                }
                void initialize() {
                    found = false;
                }
                virtual void visit(NarratorNodeIfc &, NarratorNodeIfc &, int) {    }
                virtual bool visit(NarratorNodeIfc &n) {
                    if (!found) {
                        if (&n == target) {
                            found = true;
                        }
                    }

                    return !found;
                }
                virtual void finishVisit(NarratorNodeIfc &) { }
                virtual void detectedCycle(NarratorNodeIfc &) { }
                virtual void finish() {  }
                bool isFound() {
                    return found;
                }
        };
        bool areNeighbors(NarratorNodeIfc *node1, NarratorNodeIfc *node2) {
            ReachableVisitor v(node1);
            GraphVisitorController c(&v, NULL);
            node2->BFS_traverse(c, 1, 1);

            if (v.isFound()) {
                return true;
            }

            node2->BFS_traverse(c, 1, -1);
            return v.isFound();
        }
#else
        typedef QVector<bool> BoolList;
        typedef QVector<BoolList > DoubleBoolList;
        DoubleBoolList neighbors;

        class NeighborVisitor: public NodeVisitor {
                DoubleBoolList &neighbors;
            public:
                NeighborVisitor(DoubleBoolList &n): neighbors(n) { }
                void initialize() {  }
                virtual void visit(NarratorNodeIfc &n1, NarratorNodeIfc &n2, int) {
                    int id1 = n1.getCorrespondingNarratorNode().getId();
                    int id2 = n2.getCorrespondingNarratorNode().getId();
                    neighbors[id1][id2] = true;
                    neighbors[id2][id1] = true;
                }
                virtual bool visit(NarratorNodeIfc &) {
                    return true;
                }
                virtual void finishVisit(NarratorNodeIfc &) { }
                virtual void detectedCycle(NarratorNodeIfc &) { }
                virtual void finish() {  }
        };
        void initializeNeighbors() {
            neighbors.clear();
            BoolList l;

            for (int i = 0; i < graph->size(); i++) {
                l.append(false);
            }

            for (int i = 0; i < graph->size(); i++) {
                neighbors.append(l);
            }

            NeighborVisitor v(neighbors);
            GraphVisitorController c(&v, graph, true, true);
            graph->DFS_traverse(c);
        }
        bool areNeighbors(NarratorNodeIfc *node1, NarratorNodeIfc *node2) {
            if (graph->size() != neighbors.size()) {
                initializeNeighbors();
                assert(graph->size() == neighbors.size());
            }

            int id1 = node1->getCorrespondingNarratorNode().getId();
            int id2 = node2->getCorrespondingNarratorNode().getId();
            return neighbors[id1][id2];
        }
#endif

        class ClusterGroup;
        class NodeItem {
            public:
                NarratorNodeIfc *node;
                double similarity;
                int inDegree;
                int outDegree;
                int stepsToCenter;

                NodeItem(const MatchingNode &mNode) {
                    node = mNode.node;
                    inDegree = 0;
                    outDegree = 0;
                    stepsToCenter = 0;
                    similarity = mNode.similarity;
                }
                NodeItem(NarratorNodeIfc *node, int stepsToCenter, double similarity) {
                    this->node = node;
                    this->stepsToCenter = stepsToCenter;
                    this->similarity = similarity;
                    inDegree = 0;
                    outDegree = 0;
                }
        };
        typedef QList<NodeItem *> NodeItemList;
        typedef QList<NodeItemList> NodeItemGroup;
        class ClusterGroup {
            private:
                int numNarrators;
            public:
                NarratorNodeIfc *center;
                NodeItemGroup groups;
            public:
                ClusterGroup(NarratorNodeIfc *center) {
                    this->center = center;
                    numNarrators = 0;
                }
                void addNodeItem(NodeItem *n, int narId) {
                    while (groups.size() <= narId) {
                        groups.append(NodeItemList());
                    }

                    if (groups[narId].size() == 0) {
                        numNarrators++;
                    }

                    groups[narId].append(n);
                }
                int size() {
                    return groups.size();
                }
                NodeItemList &operator[](int i) {
                    return groups[i];
                }
                int getNumNarrators() { //i.e. no empty size
                    return numNarrators;
                }
                ~ClusterGroup() {
                    int size = groups.size();

                    for (int i = 0; i < size; i++) {
                        int size = groups[i].size();

                        for (int j = 0; j < size; j++) {
                            delete groups[i][j];
                        }
                    }
                }
        };
        class ClusterList {
            private:
                QList<ClusterGroup *> list;
            public:
                void addCluster(ClusterGroup *c, int id = -1) {
                    if (id < 0) {
                        id = list.size();
                    }

                    assert(list.size() <= id || list[id] == NULL);

                    for (int i = list.size(); i <= id; i++) {
                        list.append(NULL);
                    }

                    list[id] = c;
                }
                int size() {
                    return list.size();
                }
                ClusterGroup *operator[](int i) {
                    return list[i];
                }
                ~ClusterList() {
                    for (int i = 0; i < list.size(); i++)
                        if (list[i] != NULL) {
                            delete list[i];
                        }
                }
        };
        typedef QList<NodeItem *> Cluster;
        class ClusterScore {
            public:
                int narrCount, stepCount;
                double similarityProduct;
                ClusterScore(int narr, int steps, double similarity): narrCount(narr), stepCount(steps),
                    similarityProduct(similarity) {}
        };
        class ScoredCluster {
            public:
                Cluster *cluster;
                ClusterScore score;
                NarratorNodeIfc *center;
                ScoredCluster(NarratorNodeIfc *center, int narr, int steps = 0, double similarity = 0): score(narr, steps, similarity) {
                    cluster = new Cluster;
                    this->center = center;
                }

                bool operator <(const ScoredCluster &rhs) const {
                    return (score.narrCount < rhs.score.narrCount ||
                            (score.narrCount == rhs.score.narrCount && score.stepCount > rhs.score.stepCount) ||
                            (score.narrCount == rhs.score.narrCount && score.stepCount == rhs.score.stepCount
                             && score.similarityProduct < rhs.score.similarityProduct)
                           );
                }
        };
        typedef QList<ScoredCluster> ScoredClusterList;
        class Trash { //all items saved in its structures are deleted when object goes out of scope
            public:
                QList<Cluster *> clusters;
                QList<ClusterGroup *> clusterGroups;
                ~Trash() {
                    int size = clusters.size();

                    for (int i = 0; i < size; i++) {
                        delete clusters[i];
                    }

                    size = clusterGroups.size();

                    for (int i = 0; i < size; i++) {
                        delete clusterGroups[i];
                    }
                }
        };
        class ClusteringVisitor: public NodeVisitor {
                ClusterGroup &cluster;
                NodeItemGroup &groups;
            public:
                ClusteringVisitor(ClusterGroup &aCluster, NodeItemGroup &g): cluster(aCluster), groups(g) {}
                void initialize() {  }
                virtual void visit(NarratorNodeIfc &, NarratorNodeIfc &, int) {    }
                virtual bool visit(NarratorNodeIfc &n) {
                    if (!n.hasSomeBiographyIndex()) {
                        return false;
                    }

                    int size = n.getBiographIndexCount();

                    for (int i = 0; i < size; i++) {
                        int index = n.getBiographyIndex(i);
                        double similarity = 1;
                        NodeItem *item = new NodeItem(&n, controller->getParentStack().size(), similarity);
                        cluster.addNodeItem(item, index);
                    }

                    return true;
                }
                virtual void finishVisit(NarratorNodeIfc &) { }
                virtual void detectedCycle(NarratorNodeIfc &) { }
                virtual void finish() {  }
        };
        class ClusterIterator {
            private:
                ClusterGroup &clusterGroup;
                QList<int> indicies;
                int currentIndex;
            public:
                ClusterIterator(ClusterGroup &c): clusterGroup(c) {
                    assert(c.size() > 0);

                    for (int i = 0; i < clusterGroup.size(); i++) {
                        indicies << 0;
                    }

                    currentIndex = 0;
                }
                ClusterIterator &operator++() {
                    if (clusterGroup[currentIndex].size() == 0 || indicies[currentIndex] == clusterGroup[currentIndex].size() - 1 ||
                        (clusterGroup[currentIndex].size() > 0 && clusterGroup[currentIndex][indicies[currentIndex]]->stepsToCenter == 1)
                       ) {
                        indicies[currentIndex] = 0;

                        while (++currentIndex < clusterGroup.size() && clusterGroup[currentIndex].size() == 0)
                            ;

                        if (!isFinished()) {
                            operator ++();

                            if (!isFinished()) {
                                currentIndex = 0;
                            }
                        }
                    } else {
                        indicies[currentIndex]++;
                    }

                    return *this;
                }
                bool isFinished() const {
                    assert(currentIndex <= clusterGroup.size());
                    return currentIndex == clusterGroup.size();
                }
                ScoredCluster operator*() {
                    ScoredCluster sc(clusterGroup.center, clusterGroup.getNumNarrators(), 0, 1);
                    Cluster &c = *sc.cluster;
                    ClusterScore &s = sc.score;

                    for (int i = 0; i < indicies.size(); i++) {
                        if (clusterGroup[i].size() > 0) {
                            NodeItem *n = clusterGroup[i][indicies[i]];
                            c << n;
                            s.stepCount += n->stepsToCenter;
                            s.similarityProduct *= n->similarity;
                        }
                    }

                    return sc;
                }
        };

        int getLargestClusterSize(Biography::NarratorNodeGroups &list) {
            Trash trash; //for no leakage
            NodeItemGroup nodeItemgroups;
            //1-transform to nodeItems:
            int listCount = list.size();

            for (int i = 0; i < listCount; i++) {
                int size = list[i].size();

                if (size > 0) {
                    nodeItemgroups.append(NodeItemList());

                    for (int j = 0; j < size; j++) {
                        NodeItem *node = new NodeItem(list[i][j]);
                        nodeItemgroups[i].append(node);
                        node->node->addBiographyIndex(i);
                    }

                    qSort(nodeItemgroups[i]);
                }
            }

            //find all possible clusters (from all possible cluster centers)
            ScoredCluster bestCluster(NULL, 0, (unsigned int) - 1, 0);
            int groupSize = nodeItemgroups.size();

            for (int i = 0; i < groupSize; i++) {
                for (int j = 0; j < nodeItemgroups[i].size(); j++) {
                    NarratorNodeIfc *n = nodeItemgroups[i][j]->node;
                    ClusterGroup *c = new ClusterGroup(n);
                    ClusteringVisitor v(*c, nodeItemgroups);
                    GraphVisitorController cont(&v, graph);
                    graph->DFS_traverse(*n, cont, 1, 1);
                    graph->DFS_traverse(*n, cont, 1, -1);
                    bool chosen = false;

                    if (c->getNumNarrators() >= hadithParameters.bio_narr_min) {
                        ClusterIterator itr(*c);

                        for (; !itr.isFinished(); ++itr) {
                            ScoredCluster cluster = *itr;

                            if (bestCluster < cluster) {
                                if (bestCluster.center != NULL) {
                                    assert(trash.clusters.contains(bestCluster.cluster));
                                    trash.clusters.removeOne(bestCluster.cluster);
                                    delete bestCluster.cluster;
                                }

                                bestCluster = cluster;
                                trash.clusters.append(cluster.cluster);
                                chosen = true;
                            } else {
                                delete cluster.cluster;
                            }
                        }
                    }

                    if (chosen) {
                        if (trash.clusterGroups.size() > 0) {
                            assert(trash.clusterGroups.size() == 1);
                            delete trash.clusterGroups[0];
                            trash.clusterGroups.clear();
                            trash.clusterGroups.append(c);
                        }
                    } else {
                        delete c;
                    }
                }
            }

            listCount = nodeItemgroups.size();

            for (int i = 0; i < listCount; i++) {
                NodeItemList &l = nodeItemgroups[i];
                int size = l.size();

                for (int j = 0; j < size; j++) {
                    l[j]->node->clearBiographyIndicies();
                }
            }

            if (bestCluster.center == NULL) {
                return 0;
            }

            ScoredCluster &c = bestCluster;
            qDebug() << "[" << c.center->CanonicalName() << "]\n(" << c.score.narrCount << "," << c.score.stepCount << "," <<
                     c.score.similarityProduct << ")\n";

            for (int i = 0; i < c.cluster->size(); i++) {
                qDebug() << (*c.cluster)[i]->node->CanonicalName() << "\n";
            }

            qDebug() << "\n";
            return c.score.narrCount;
        }
#else
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

#endif

#endif
        bool tagNarrator(const Narrator *n, bool /*isReal=true*/) {
            if (n->m_narrator.size() == 0) {
                return false;
            }

#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY

            if (!isReal) {
                prg->tag(n->getStart(), n->getLength(), Qt::darkGray, false);
            } else
#endif
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
#ifdef SEGMENT_BIOGRAPHY_USING_POR
        NarratorDetector(NarratorGraph *graph)
#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
            : hash(graph)
#endif
        {
            this->graph = graph;
            biographies = NULL;
        }
        NarratorDetector()
#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
            : hash(NULL)
#endif
        {
            biographies = NULL;
        }
#else
        NarratorDetector() {
            graph = NULL;
            biographies = NULL;
        }
#endif
        int lookup(QString input_str, ATMProgressIFC *prg)  {
            this->prg = prg;
            QFile chainOutput(chainDataStreamFileName);
            chainOutput.remove();

            if (!chainOutput.open(QIODevice::ReadWrite)) {
                return 1;
            }

#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
            QDataStream chainOut(&chainOutput);
#endif
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
#ifdef CHAIN_BUILDING
            HadithData *currentBiography = new HadithData(text, false, graph, input_str);
            currentBiography->initialize(text);
            currentBiography->learningEvaluator.resetLearnedNames();
            //display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
#else
            chainData *currentBiography = NULL;
#endif
#if !defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY)
            long  biographyStart = -1;
            long  biographyEnd;
            int biography_Counter = 1;
#endif
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
#if defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY) && defined(SEGMENT_BIOGRAPHY_USING_POR)
                    addNarrators(currentBiography->biography);
#else
#ifdef SEGMENT_BIOGRAPHY_USING_POR
                    Biography::NarratorNodeGroups &realNarrators = currentBiography->biography->nodeGroups;
                    int num = getLargestClusterSize(realNarrators);

                    if (num >= hadithParameters.bio_narr_min) {
#else

                    if (currentData.narratorCount >= hadithParameters.bio_narr_min) {
#endif
                        //biographyEnd=currentData.narratorEndIndex;
                        biographyEnd = stateInfo.endPos;
                        currentBiography->biography->setEnd(biographyEnd);
#ifdef DISPLAY_HADITH_OVERVIEW
                        //biographyStart=currentData.biographyStartIndex;
                        biographyStart = currentBiography->biography->getStart();
                        //long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
                        //long end=stateInfo.endPos;
                        out << "\n" << biography_Counter << " new biography start: " << text->mid(biographyStart, display_letters) << endl;
                        out << "sanad end: " << text->mid(biographyEnd - display_letters + 1, display_letters) << endl << endl;
#ifdef CHAIN_BUILDING
                        currentBiography->biography->serialize(chainOut);
                        currentBiography->biography->setStart(stateInfo.nextPos);
                        //currentChain->chain->serialize(displayed_error);
#endif
#endif
                        biography_Counter++;
                    } else {
                        delete currentBiography->biography;
                    }

#endif
                }

#ifdef SEGMENT_BIOGRAPHY_USING_POR
                additionalCheck(stateInfo);
#endif
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
#if defined(DISPLAY_HADITH_OVERVIEW) && !defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY)

            if (biographyStart < 0) {
                out << "no biography found\n";
                chainOutput.close();
                prg->startTaggingText(*text);
                prg->finishTaggingText();
                return 2;
            }

            chainOutput.close();
#endif
#ifdef CHAIN_BUILDING
            int tester_Counter = 1;
#ifdef TEST_BIOGRAPHIES
            biographies = new BiographyList;
            biographies->clear();
#endif
#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY

            if (!chainOutput.open(QIODevice::ReadWrite)) {
                return 1;
            }

            QDataStream tester(&chainOutput);
#else
            modifyNodes();
#endif
            prg->startTaggingText(*text);
#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY

            while (!tester.atEnd()) {
                Biography *s = new Biography(graph, text);
                s->deserialize(tester);
#ifdef TEST_BIOGRAPHIES
                biographies->append(s);
#endif

                for (int j = 0; j < s->size(); j++) {
                    const Narrator *n = (*s)[j];
                    bool isReal = s->isReal(j);
#else
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
#endif

                    if (!tagNarrator(n, isReal)) {
                        theSarf->out << "found a problem an empty narrator in (" << tester_Counter << "," << j << ")\n";
                    }
                }

                tester_Counter++;
            }

#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
        }

        prg->setCurrentAction("Looking up Biographies");
        prg->report((double)i / nodesSize * 100 + 0.5);
}

prg->report(100);
prg->setCurrentAction("Complete");
#else
            chainOutput.close();
#endif
#endif
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
#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY

    for (int i = 0; i < narratorList.size(); i++) {
        delete narratorList[i];
    }

#endif
}
};

#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
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

#endif

int biographyHelper(QString input_str, ATMProgressIFC *prg) {
    input_str = input_str.split("\n")[0];
    NarratorDetector s;
    s.lookup(input_str, prg);
#ifdef TEST_BIOGRAPHIES
    s.freeMemory();
#endif
    return 0;
}

#ifdef TEST_BIOGRAPHIES
BiographyList *getBiographies(QString input_str, NarratorGraph *graph, ATMProgressIFC *prg, int /*nodeID*/) {
    input_str = input_str.split("\n")[0];
#ifdef BIOGRAPHY_SEGMENT
    BiographySegmenter s(graph);
    s.segment(input_str, prg);
#else
#ifdef SEGMENT_BIOGRAPHY_USING_POR
    NarratorDetector s(graph);
#else
    NarratorDetector s;
#endif

    if (nodeID >= 0) {
        NarratorNodeIfc *n = graph->getNode(nodeID);
        NarratorNodesList list;
        list.append(n);
        s.nodes.append(list);
    }

    s.lookup(input_str, prg);
#endif
    return s.biographies;
}
#endif
