#ifndef GRAPH_STRUCTURE_H
#define GRAPH_STRUCTURE_H

#include "Triplet.h"
#include <assert.h>
#include <QList>
#include <QMap>
#include <QPair>
#include <QFile>
#include "logger.h"
#include <QDebug>
#include <QQueue>
#include <QDataStream>
#include "reference.h"
#include "narrator_abstraction.h"
#include "hadith_utilities.h"

#include "hadith.h"

class NarratorNodeIfc;
class ChainNarratorNode;
class ChainNarratorNode;
class GraphNarratorNode;
class GroupNode;
class NULLGraphNarratorNode;
class NULLNarratorNodeIfc;
class NULLChainNarratorNode;
class NodeIterator;
class GraphNodeItem;
class NarratorGraph;


typedef QPair<NarratorNodeIfc &, ChainNarratorNode &> NodeAddress;
typedef QList<int> BiographyIndicies;

extern NodeAddress nullNodeAddress;
extern NULLNarratorNodeIfc nullNarratorNodeIfc;
extern NULLGraphNarratorNode nullGraphNarratorNode;
extern NULLChainNarratorNode nullChainNarratorNode;

class RankCorrectorNodeVisitor;
class NarratorGraph;
class GraphVisitorController;
class ColorIndices;



class NarratorNodeIfc { //abstract interface
    private:
        unsigned int id;
        BiographyIndicies indicies;
        unsigned int color;
        friend class LoopBreakingVisitor;

    protected:
        virtual void setRank(int rank) = 0;
        virtual int getSavedRank() const = 0;
        virtual int getAutomaticRank() const = 0;
        friend class RankCorrectorNodeVisitor;

        virtual void setVisited(unsigned int bit) {
            color |= 1 << bit;
        }
        virtual void resetVisited(unsigned int bit) {
            color &= (~(1 << bit));
        }
        virtual void resetColor() {
            color = 0;
        }
        NarratorNodeIfc(NarratorGraph &g);
        virtual ~NarratorNodeIfc() {}
        friend class NarratorGraph;
        friend class ColorIndices;
        friend class GraphVisitorController;

        virtual void serializeHelper(QDataStream &chainOut, NarratorGraph &graph) const = 0;
        virtual void deserializeHelper(QDataStream &chainIn, NarratorGraph &graph) = 0;


        friend class NodeIterator;
        virtual NarratorNodeIfc &getChild(int index1, int index2) = 0;
        virtual NarratorNodeIfc &getParent(int index1, int index2) = 0;
        virtual NarratorNodeIfc &getChild(const QPair<int, int> &p) {
            return getChild(p.first, p.second);
        }
        virtual NarratorNodeIfc &getParent(const QPair<int, int> &p) {
            return getParent(p.first, p.second);
        }

        NarratorNodeIfc() {
            resetColor();
            id = -1;
        }
    public:
        virtual NarratorNodeIfc &getCorrespondingNarratorNode() = 0;  //if used on a graphNode returns null

        virtual int size() const = 0;
        virtual int getNumChildren() const {
            return size();
        }
        virtual int getNumParents() const {
            return size();
        }

        virtual GraphNodeItem &operator [](int index) = 0;
        NodeIterator begin();
        virtual NodeIterator childrenBegin();
        virtual NodeIterator parentsBegin();
        void setId(unsigned int aId) {
            id = aId;
        }
        unsigned int  getId() const {
            return id;
        }
        //virtual NodeAddress prevInChain(ChainNarratorNode &)=0;
        //virtual NodeAddress nextInChain(ChainNarratorNode &) =0;

        virtual QString CanonicalName() const = 0;
        virtual bool isNull() const = 0;
        virtual bool isGraphNode() const {
            return false;
        }
        virtual bool isChainNode() const {
            return false;
        }
        virtual bool isGroupNode() const {
            return false;
        }
        virtual bool isActualNode() const {
            return false;
        }

        virtual int getRank() const {
            int savedRank = getSavedRank();
            return (savedRank >= 0 ? savedRank : getAutomaticRank());
        }
        virtual QString rank2String() const = 0;

        virtual ChainNarratorNode &getChainNodeInChain(int chain_num) = 0;
        virtual QString toString() const = 0;
        virtual bool isVisited(unsigned int bit) const {
            return (color & (1 << bit)) != 0;
        }

        void addBiographyIndex(int i) {
            NarratorNodeIfc *n = &getCorrespondingNarratorNode();

            if (n == NULL) {
                n = (NarratorNodeIfc *)&operator [](0);
            }

            if (!n->indicies.contains(i)) {
                n->indicies.append(i);
                /*qDebug()<<(long)this<<"-"<<(long)n
                  <<"<"<<n->CanonicalName()<<">:"<<i;*/
            }
        }
        int getBiographIndexCount() {
            NarratorNodeIfc &n = getCorrespondingNarratorNode();
            return (n.indicies.size());
        }
        int getBiographyIndex(int i) {
            NarratorNodeIfc &n = getCorrespondingNarratorNode();
            return (n.indicies[i]);
        }
        bool hasBiographyIndex(int i) {
            NarratorNodeIfc &n = getCorrespondingNarratorNode();
            /*qDebug()<<"---"<<(long)&n<<"<"<<CanonicalName()<<">---";
              for (int j=0;j<n.indicies.size();j++)
              qDebug()<<n.indicies[j];*/
            return (n.indicies.contains(i));
        }
        bool hasSomeBiographyIndex() {
            NarratorNodeIfc &n = getCorrespondingNarratorNode();
            return (!n.indicies.isEmpty());
        }
        void clearBiographyIndicies() {
            indicies.clear();
        }
        void serialize(QDataStream &chainOut, NarratorGraph &graph) const {
            chainOut << isNull();
            chainOut << isGraphNode();
            chainOut << isChainNode();

            if (!isNull()) {
                chainOut << indicies;
                serializeHelper(chainOut, graph);
            }
        }
        static NarratorNodeIfc *deserialize(QDataStream &chainIn, NarratorGraph &graph) ;

        void BFS_traverse(GraphVisitorController &visitor, int maxLevels = -1, int direction = 1);


};

class NULLNarratorNodeIfc: public NarratorNodeIfc {
        virtual NarratorNodeIfc &getCorrespondingNarratorNode() {
            assert(false);
        }

        int size() const {
            assert(false);
        }
        NarratorNodeIfc &getChild(int, int) {
            assert(false);
        }
        NarratorNodeIfc &getParent(int, int) {
            assert(false);
        }
        GraphNodeItem &operator [](int) {
            assert(false);
        }

        //ChainNarratorNode & firstChainNode() {assert(false);}
        //ChainNarratorNode & nextChainNode(ChainNarratorNode & ) {assert(false);}

        //NodeAddress prevInChain(ChainNarratorNode &) {assert(false);}
        //NodeAddress nextInChain(ChainNarratorNode &) {assert(false);}

        QString CanonicalName() const {
            assert(false);
        }

        virtual bool isNull() const {
            return true;
        }
        void setRank(int) {
            assert(false);
        }
        int getAutomaticRank() const {
            assert(false);
        }
        int getRank() const {
            assert(false);
        }
        int getSavedRank() const {
            assert(false);
        }
        virtual QString rank2String() const {
            assert(false);
        }
        virtual ChainNarratorNode &getChainNodeInChain(int) {
            assert(false);
        }
        virtual QString toString() const   {
            return "NULLNarratorNodeIfc";
        }
        void setVisited(unsigned int) {
            assert(false);
        }
        void resetVisited(unsigned int) {
            assert(false);
        }
        bool isVisited(unsigned int) const {
            assert(false);
        }
        void resetColor() {
            assert(false);
        }

        virtual void serializeHelper(QDataStream &, NarratorGraph &) const {}
        virtual void deserializeHelper(QDataStream &, NarratorGraph &) {}
};

class GraphNodeItem: public NarratorNodeIfc { //abstract class
    protected:
        QString key;
        GraphNodeItem() {}
    public:
        QString getKey() {
            assert(!isNull());
            return key;
        }
        GraphNodeItem(NarratorGraph &g): NarratorNodeIfc(g) {}
};

class ChainNarratorNode: public GraphNodeItem {
    private:
        class ChainContext {
            private:
                bool valid: 1; //to indicate if info in the variable is valid
                int index: 8;
                int chain_num: 23;
            public:
                ChainContext() :
                    valid(false),
                    index(0),
                    chain_num(0) {}

                ChainContext(int index, int chain_num) {
                    set(index, chain_num);
                }

                void set(int index, int chain_num) {
                    //assert(index<20);
                    this->index = index;
                    this->chain_num = chain_num;
                    valid = true;
                }
                int getIndex() const {
                    assert(valid);
                    return index;
                }
                int getChainNum() const {
                    assert(valid);
                    return chain_num;
                }
                void printChainContext() { //for debugging purposes only
                    theSarf->out << chain_num << "[" << index << "]\n";
                    //qDebug()<<chain_num<<"["<<index<<"]\n";;
                }
        };

    private:
        ChainNarratorNode *previous, * next;
        Narrator *narrator;

        GroupNode *group;
        int savedRank;
        friend class NarratorGraph;
        friend class GroupNode;

        ChainContext chainContext;
        friend void buildChainNodes(ChainsContainer &chains);
    protected:

        ChainNarratorNode(): GraphNodeItem() { //to be used by NULLChainNarratorNode
            previous = next = NULL;
            group = NULL;
            narrator = NULL;
            savedRank = -1;
            group = NULL;
            chainContext.set(-1, -1);
        }
        friend class NarratorNodeIfc;
        friend class NarratorHash;

        virtual int getSavedRank() const ;
        virtual int getAutomaticRank() const {
            int savedRank = getSavedRank();
            return (savedRank >= 0 ? savedRank : getIndex());
        }
        virtual void setRank(int rank);
        virtual void setIndex(int index) {
            chainContext.set(index, chainContext.getChainNum());
        }
        virtual void setChainNum(int num) {
            chainContext.set(chainContext.getIndex(), num);
        }
        friend class GraphNarratorNode;

        virtual void serializeHelper(QDataStream &chainOut, NarratorGraph &graph) const;
        virtual void deserializeHelper(QDataStream &chainIn, NarratorGraph &graph) ;
    public:

        ChainNarratorNode(NarratorGraph &g, Narrator *n, int index, int chain_num)
            : GraphNodeItem(g) {
            previous = next = NULL;
            group = NULL;
            narrator = n;
            key = n->getKey();
            savedRank = -1;
            group = NULL;
            chainContext.set(index, chain_num);
        }
        virtual NarratorNodeIfc &getCorrespondingNarratorNode();
        virtual void  setCorrespondingNarratorNodeGroup(GroupNode *group) {
            this->group = group;
        }
        virtual QString toString() const {
            return "(" + CanonicalName() + ")";
        }
        virtual Narrator &getNarrator() const {
            return (Narrator &) * narrator;
        }
        virtual ChainNarratorNode &operator+(int n) {
            if (n >= 0) {
                ChainNarratorNode *c = (ChainNarratorNode *)this;

                for (int i = 0; i < n && !c->isNull(); i++) {
                    c = &(c->nextInChain());
                }

                return *c;
            } else {
                ChainNarratorNode *c = (ChainNarratorNode *)this;

                for (int i = 0; i > n && !c->isNull(); i--) {
                    c = &(c->prevInChain());
                }

                return *c;
            }
        }
        virtual ChainNarratorNode &operator-(int n)  {
            return this->operator +(-n);
        }
        virtual ChainNarratorNode &prevInChain() ;
        virtual ChainNarratorNode &nextInChain() ;
        virtual NarratorNodeIfc &nextChild(NarratorNodeIfc &)  {
            return nullNarratorNodeIfc;
        }
        virtual int size() const {
            return 1;
        }
        virtual NarratorNodeIfc &getChild(int index1, int index2);
        virtual NarratorNodeIfc &getParent(int index1, int index2);
        virtual ChainNarratorNode &operator [](int index) {
            assert(index == 0);
            return *this;
        }
        virtual NodeAddress prevInChain(ChainNarratorNode &node) {
            assert(&node == this);
            ChainNarratorNode &prev = node.prevInChain();

            if (!prev.isNull()) {
                return NodeAddress(prev.getCorrespondingNarratorNode(), prev);
            } else {
                return nullNodeAddress;
            }
        }
        virtual NodeAddress nextInChain(ChainNarratorNode &node) {
            assert(&node == this);
            ChainNarratorNode &next = node.nextInChain();

            if (!next.isNull()) {
                return NodeAddress(next.getCorrespondingNarratorNode(), next);
            } else {
                return nullNodeAddress;
            }
        }
        virtual QString CanonicalName() const {
            return narrator->getString().replace("\n", " ");
        }
        virtual bool isFirst() const {
            return previous == NULL;
        }
        virtual bool isLast() const {
            return next == NULL;
        }
        virtual int getIndex() const {
            return chainContext.getIndex();
        }
        virtual QString rank2String() const {
            return QString("(%1)").arg(getSavedRank());
        }
        virtual int getChainNum() const {
            return chainContext.getChainNum();
        }
        virtual bool isNull() const {
            return false;
        }
        virtual bool isChainNode() const {
            return true;
        }
        virtual bool isActualNode() const;
        virtual ChainNarratorNode &getChainNodeInChain(int chain_num);
        virtual GroupNode &getGroupNode() {
            return *group;
        }
};

class NULLChainNarratorNode: public ChainNarratorNode {
    protected:
        int getSavedRank() const {
            assert(false);
        }
        int getAutomaticRank() const {
            assert(false);
        }
        void setRank(int) {
            assert(false);
        }
        void setIndex(int) {
            assert(false);
        }

        virtual void serializeHelper(QDataStream &, NarratorGraph &) const {}
        virtual void deserializeHelper(QDataStream &, NarratorGraph &) {}
    public:
        NULLChainNarratorNode() {}
        virtual ~NULLChainNarratorNode() {}
        bool isNull() const {
            return true;
        }
        NarratorNodeIfc &getCorrespondingNarratorNode() {
            assert(false);
        }
        void  setCorrespondingNarratorNodeGroup(GroupNode *) {
            assert(false);
        }
        Narrator &getNarrator() const {
            assert(false);
        }
        ChainNarratorNode &operator+(int) {
            assert(false);
        }
        ChainNarratorNode &operator-(int) {
            assert(false);
        }
        int size() const {
            return 0;
        }
        NarratorNodeIfc &getChild(int, int) {
            assert(false);
        }
        NarratorNodeIfc &getParent(int, int) {
            assert(false);
        }
        ChainNarratorNode &operator [](int) {
            assert(false);
        }
        NodeAddress prevInChain(ChainNarratorNode &) {
            assert(false);
        }
        NodeAddress nextInChain(ChainNarratorNode &) {
            assert(false);
        }
        ChainNarratorNode &prevInChain() {
            assert(false);
        }
        ChainNarratorNode &nextInChain() {
            assert(false);
        }
        bool isFirst() const {
            assert(false);
        }
        bool isLast() const {
            assert(false);
        }
        int getIndex() const {
            assert(false);
        }
        QString rank2String() const {
            assert(false);
        }
        QString CanonicalName() const {
            assert(false);
        }
        int getChainNum() const {
            assert(false);
        }
        ChainNarratorNode &getChainNodeInChain(int) {
            assert(false);
        }
        QString toString() const {
            return "NULLChainNarratorNodeIterator";
        }
        void setVisited(unsigned int) {
            assert(false);
        }
        void resetVisited(unsigned int) {
            assert(false);
        }
        bool isVisited(unsigned int)  const {
            assert(false);
        }
        void resetColor() {
            assert(false);
        }
};

class GroupNode: public GraphNodeItem {
    public:
        typedef Triplet<QString, int, int> KeyInfo; //(key, value,total)
        typedef QList<KeyInfo> KeyList;
    private:
        GraphNarratorNode *graphNode;
        QList<ChainNarratorNode *> list;
        KeyList allKeys;
        int lowestIndex: 16, highestIndex: 16;
    private:
        friend class GraphNarratorNode;
        friend class ChainNarratorNode;
        friend class NarratorNodeIfc;
        void setRank(int rank);
    protected:
        friend class NarratorHash;
        friend class NarratorGraph;
        GroupNode(): GraphNodeItem() {
            graphNode = NULL;
            lowestIndex = -1;
            highestIndex = -1;
        }
        GroupNode(NarratorGraph &g, GraphNarratorNode *gNode, ChainNarratorNode *cNode, QString key);

        GroupNode(NarratorGraph &g, GraphNarratorNode *gNode, ChainNarratorNode *cNode);

        virtual NarratorNodeIfc &getChild(int index1, int) {
            assert(false);
            return list[index1]->getChild(0, 0);
        }
        virtual NarratorNodeIfc &getParent(int index1, int) {
            return list[index1]->getParent(0, 0);
        }
        virtual NarratorNodeIfc &getChild(const QPair<int, int> &p) {
            return getChild(p.first, p.second);
        }
        virtual NarratorNodeIfc &getParent(const QPair<int, int> &p) {
            return getParent(p.first, p.second);
        }

    public:
        const KeyList &getKeyList() const {
            return allKeys;
        }
        ChainNarratorNode &operator [](int index) {
            assert(index >= 0 && index < size()); //check redundant will be done inside [] for QList
            return *list[index];
        }
        void addChainNode(NarratorGraph *g, ChainNarratorNode &nar);   //we dont check for duplicates here
        int getLowestIndex() {
            return lowestIndex;
        }
        int getHighestIndex() {
            return highestIndex;
        }
        virtual NarratorNodeIfc &getCorrespondingNarratorNode();
        int size() const {
            return list.size();
        }
        QString CanonicalName() const {
            if (isRasoul(list[0]->CanonicalName())) {
                return alrasoul;
            }

            int smallestsize = list[0]->CanonicalName().size(), index = 0;

            for (int i = 1; i < list.size(); i++) {
                int size = list[i]->CanonicalName().size();

                if (smallestsize > size) {
                    smallestsize = size;
                    index = i;
                }
            }

            if (index >= 0) {
                return list[index]->CanonicalName();
            } else {
                return "";
            }
        }
        ChainNarratorNode &getChainNodeInChain(int chain_num) {
            for (int i = 0; i < list.size(); i++)
                if (list[i]->getChainNum() == chain_num) {
                    return *list[i];
                }

            return nullChainNarratorNode;
        }
        QString toString() const {
            QString s = QString("<");

            for (int i = 0; i < list.size(); i++) {
                s += list[i]->toString();
            }

            s += ">";
            return s;
        }
        QString getKey() {
            return key;
        }
        int getSavedRank() const;
        int getAutomaticRank() const {
            int smallest_rank = list[0]->getAutomaticRank();

            for (int i = 1; i < list.size(); i++) {
                int rank = list[i]->getAutomaticRank();

                if (smallest_rank < rank) {
                    smallest_rank = rank;
                }
            }

            return smallest_rank;
        }
        void setGraphNode(GraphNarratorNode *node) {
            //assert(node==NULL || !((NarratorNodeIfc*)node)->isGroupNode());
            this->graphNode = node;
        }
        virtual bool isNull() const {
            return false;
        }

        virtual void serializeHelper(QDataStream &chainOut, NarratorGraph &graph) const;
        virtual void deserializeHelper(QDataStream &chainIn, NarratorGraph &graph);

        virtual QString rank2String() const {
            QString ranks;
            ranks += QString("(%1)").arg(getSavedRank());
            return ranks;
        }
        virtual bool isGroupNode() const {
            return true;
        }
};

class GraphNarratorNode: public NarratorNodeIfc {
    public:
        typedef QList<NarratorNodeIfc *> NodeList;
        int lowestIndex: 16, highestIndex: 16;
    private:

        void serializeCache(QDataStream &chainIn, NarratorGraph &graph, const NodeList &list) const;
        void deserializeCache(QDataStream &chainIn, NarratorGraph &graph, NodeList &list);

        friend class GraphVisitorController;
        friend class HadithTaggerDialog;
        bool fillChildren();
        bool fillParents();
        void clearCache() {
            children.clear();
            parents.clear();
        }
    protected:
        friend class NodeIterator;
        NodeList parents, children;

        friend class NarratorGraph;
        QList<GroupNode *>  groupList;
        int savedRank;
        virtual void setRank(int rank) {
            savedRank = rank;
        }
        virtual int getAutomaticRank() const {
            int smallest_rank = groupList[0]->getAutomaticRank();

            for (int i = 1; i < groupList.size(); i++) {
                int rank = groupList[i]->getAutomaticRank();

                if (smallest_rank < rank) {
                    smallest_rank = rank;
                }
            }

            return smallest_rank;
        }
        virtual int getSavedRank() const {
            return savedRank;
        }
        friend class LoopBreakingVisitor;

        virtual void serializeHelper(QDataStream &chainOut, NarratorGraph &graph) const;
        virtual void deserializeHelper(QDataStream &chainIn, NarratorGraph &graph) ;

        GraphNarratorNode(): NarratorNodeIfc() { //to be used by NULLGraphNarratorNode
            savedRank = -1;
            lowestIndex = -1;
            highestIndex = -1;
        }
        friend class NarratorNodeIfc;
        friend class GroupNode;
    public:
        GraphNarratorNode(NarratorGraph &g, ChainNarratorNode &nar1, ChainNarratorNode &nar2)
            : NarratorNodeIfc(g) {
            assert(nar1.group == NULL);
            assert(nar2.group == NULL);
            assert(&nar1 != &nar2); //make sure these are not just the same node
            lowestIndex = nar1.getIndex();
            highestIndex = lowestIndex;
            addChainNode(&g, nar1);
            addChainNode(&g, nar2);
            savedRank = -1;
        }
        GraphNarratorNode(NarratorGraph &graph, GroupNode &g)
            : NarratorNodeIfc(graph) {
            assert(g.graphNode == NULL || g.graphNode->isChainNode());
            assert(g.size() > 1); //not allowed to have just one chain node
            groupList.append(&g);
            g.setGraphNode(this);
            lowestIndex = g.getLowestIndex();
            highestIndex = g.getHighestIndex();
            savedRank = -1;
        }
        GraphNarratorNode(NarratorGraph &graph, GroupNode &g1, GroupNode &g2)
            : NarratorNodeIfc(graph) {
            assert(g1.graphNode == NULL || g1.graphNode->isChainNode());
            assert(g2.graphNode == NULL || g2.graphNode->isChainNode());
            assert(&g1 != &g2); //make sure these are not just the same node
            groupList.append(&g1);
            groupList.append(&g2);
            g1.setGraphNode(this);
            g2.setGraphNode(this);
            lowestIndex = min(g1.getLowestIndex(), g2.getLowestIndex());
            highestIndex = max(g1.getHighestIndex(), g2.getHighestIndex());
            savedRank = -1;
        }
        virtual void addChainNode(NarratorGraph *g, ChainNarratorNode &nar) {  //we dont check for duplicates here
            QString key = nar.getKey();

            for (int i = 0; i < size(); i++) {
                GroupNode &group = (*this)[i];

                if (group.getKey() == key) {
                    group.addChainNode(g, nar);
                    return;
                }
            }

            GroupNode *newGoup = new GroupNode(*g, this, &nar, key);
            groupList.append(newGoup);
            int index = nar.getIndex();

            if (lowestIndex < 0 || lowestIndex > index) {
                lowestIndex = index;
            }

            if (highestIndex < index) {
                highestIndex = index;
            }
        }
        void addGroupNode(GroupNode &group) {  //we dont check for duplicates here
            groupList.append(&group);
            group.setGraphNode(this);
            int lIndex = group.getLowestIndex(),
                hIndex = group.getHighestIndex();

            if (lowestIndex < 0 || lowestIndex > lIndex) {
                lowestIndex = lIndex;
            }

            if (highestIndex < hIndex) {
                highestIndex = hIndex;
            }
        }
        int getLowestIndex() {
            return lowestIndex;
        }
        int getHighestIndex() {
            return highestIndex;
        }
        virtual NodeIterator childrenBegin();
        virtual NodeIterator parentsBegin();
        virtual NarratorNodeIfc &getCorrespondingNarratorNode() {
            return *this;
        }
        virtual int size() const {
            return groupList.size();
        }
        virtual int getNumChildren() const { //may result in a problem if called while 'children' is being filled
            if (children.isEmpty()) {
                return size();
            } else {
                return children.size();
            }
        }
        virtual int getNumParents() const {
            if (parents.isEmpty()) {
                return size();
            } else {
                return parents.size();
            }
        }
        virtual NarratorNodeIfc &getChild(int index1, int index2) {
            assert(index1 >= 0 && index1 < size()); //check redundant will be done inside [] for QList
            assert(index2 >= 0 && index2 < groupList[index1]->size());
            ChainNarratorNode &c = (*groupList[index1])[index2].nextInChain();
            return (c.isNull()
                    ? nullChainNarratorNode
                    : c.getCorrespondingNarratorNode());
        }
        virtual NarratorNodeIfc &getParent(int index1, int index2) {
            assert(index1 >= 0 && index1 < size()); //check redundant will be done inside [] for QList
            assert(index2 >= 0 && index2 < groupList[index1]->size());
            ChainNarratorNode &c = (*groupList[index1])[index2].prevInChain();
            return (c.isNull()
                    ? nullChainNarratorNode
                    : c.getCorrespondingNarratorNode());
        }
        virtual GroupNode &operator [](int index) {
            assert(index >= 0 && index < size()); //check redundant will be done inside [] for QList
            return *groupList[index];
        }
        virtual NodeAddress prevInChain(ChainNarratorNode
                                        &node) { //does not check if node belongs to graph node, since will work in all cases
            assert(!node.isNull());
            ChainNarratorNode &prev = node.prevInChain();
            return (prev.isNull()
                    ? nullNodeAddress
                    : NodeAddress(prev.getCorrespondingNarratorNode(), prev));
        }
        virtual NodeAddress nextInChain(ChainNarratorNode
                                        &node) { //does not check if node belongs to graph node, since will work in all cases
            assert(!node.isNull());
            ChainNarratorNode &next = node.nextInChain();
            return (next.isNull()
                    ? nullNodeAddress
                    : NodeAddress(next.getCorrespondingNarratorNode(), next));
        }
        virtual QString rank2String() const {
            QString ranks;
            ranks += QString("(%1)").arg(getSavedRank());
            return ranks;
        }
        virtual QString CanonicalName() const {
            if (isRasoul(groupList[0]->CanonicalName())) {
                return alrasoul;
            }

            //qDebug()<<"---";
            int smallestsize = groupList[0]->CanonicalName().size(), index = 0;

            //qDebug()<<"("<<groupList[0]->CanonicalName();
            for (int i = 1; i < groupList.size(); i++) {
                int size = groupList[i]->CanonicalName().size();

                //qDebug()<<groupList[i]->CanonicalName();
                if (smallestsize > size) {
                    smallestsize = size;
                    index = i;
                }
            }

            if (index >= 0) {
                //qDebug()<<")=>{"<<groupList[index]->CanonicalName()<<"}";
                return groupList[index]->CanonicalName();
            } else {
                return "";
            }
        }
        virtual bool isGraphNode() const {
            return true;
        }
        virtual bool isActualNode() const {
            return true;
        }
        virtual ChainNarratorNode &getChainNodeInChain(int chain_num) {
            for (int i = 0; i < groupList.size(); i++) {
                for (int j = 0; j < groupList[i]->size(); j++) {
                    ChainNarratorNode &c = (*groupList[i])[j];

                    if (c.getChainNum() == chain_num) {
                        return c;
                    }
                }
            }

            return nullChainNarratorNode;
        }
        virtual QString toString() const {
            QString s = QString("[");

            for (int i = 0; i < groupList.size(); i++) {
                s += groupList[i]->toString();
            }

            s += "]";
            return s;
        }
        virtual bool isNull() const {
            return false;
        }
};

class NULLGraphNarratorNode: public GraphNarratorNode {
    protected:
        int getSavedRank() const {
            assert(false);
        }
        int getAutomaticRank() const {
            assert(false);
        }
        void setRank(int) {
            assert(false);
        }
        virtual void serializeHelper(QDataStream &, NarratorGraph &) const {}
        virtual void deserializeHelper(QDataStream &, NarratorGraph &) {}
    public:
        NULLGraphNarratorNode(): GraphNarratorNode() {}
        virtual ~NULLGraphNarratorNode() {}
        bool isNull() const {
            return true;
        }
        void addChainNode(NarratorGraph *, ChainNarratorNode &) {
            assert(false);
        }
        virtual NarratorNodeIfc &getCorrespondingNarratorNode() {
            assert(false);
        }
        int size() const {
            return 0;
        }
        virtual NarratorNodeIfc &getChild(int, int) {
            assert(false);
        }
        NarratorNodeIfc &getParent(int, int) {
            assert(false);
        }
        ChainNarratorNode &operator [](int) const {
            assert(false);
        }
        NodeAddress prevInChain(ChainNarratorNode &) {
            assert(false);
        }
        NodeAddress nextInChain(ChainNarratorNode &) {
            assert(false);
        }
        QString rank2String() const {
            assert(false);
        }
        QString CanonicalName() const {
            assert(false);
        }
        ChainNarratorNode &getChainNodeInChain(int) {
            assert(false);
        }
        QString toString() const {
            return "NULLChainNarratorNodeIterator";
        }
        void setVisited(unsigned int) {
            assert(false);
        }
        void resetVisited(unsigned int) {
            assert(false);
        }
        bool isVisited(unsigned int) const {
            assert(false);
        }
        void resetColor() {
            assert(false);
        }
};

class NodeIterator {
    protected:
        typedef enum {CHILDREN, PARENTS, CHAINS, UNDEFINED_TYPE} Type;
        NarratorNodeIfc *node;
        int i: 30;
        Type type: 2;
        union {
            int j;
            GraphNarratorNode::NodeList *list;
        };
    private:
        GraphNarratorNode *gn() {
            return dynamic_cast<GraphNarratorNode *>(node);
        }

        friend class NarratorNodeIfc;
        NodeIterator(NarratorNodeIfc *n): node(n) { //unfilled type
            if (n != NULL) {
                //assert(!n->isGroupNode()); no problem in being group node, it will return its chain nodes
                i = 0;
                j = 0;
                type = CHAINS;
            } else {
                i = -1;
                j = -1;
                type = UNDEFINED_TYPE;
            }
        }

        friend class GraphNarratorNode;
        NodeIterator(NarratorNodeIfc *n, bool child): node(n) { //(Parent or Child) and GraphNode
            //if it is chainNode, we call first constructor
            assert(n->isGraphNode());
            type = (child ? CHILDREN : PARENTS);
            i = 0;
            list = &(child ? gn()->children : gn()->parents);
        }

    public:
        NodeIterator &operator++() {
            if (type == CHAINS) {
                if (i >= node->size()) {
                    return *this;
                }

                j++;

                if (j == (*node)[i].size()) {
                    j = 0;
                    i++;
                }

                return *this;
            } else {
                i++;
                return *this;
            }
        }
        bool isFinished() const {
            if (type == CHAINS) {
                return (i >= node->size());
            } else {
                return (i >= list->size());
            }
        }
        NarratorNodeIfc &operator * () {  //if we are using a chain iterator means get chain else means get parent or child
            if (type == CHAINS) {
                return (ChainNarratorNode &)(*node)[i][j];
            } else {
                return *(*list)[i];
            }
        }
        NarratorNodeIfc *getNode() {
            return &operator *();
        }
        NarratorNodeIfc &getChild() {
            if (type == CHAINS) {
                return node->getChild(i, j);
            }

            assert(type == CHILDREN);
            return operator *();
        }
        NarratorNodeIfc &getParent() {
            if (type == CHAINS) {
                return node->getParent(i, j);
            }

            assert(type == PARENTS);
            return operator *();
        }
        int getGroupIndex() const {
            assert(type == CHAINS);
            return i;
        }
        int getChainIndex() const {
            assert(type == CHAINS);
            return j;
        }
        int getIndex() const {
            assert(type != CHAINS);
            return i;
        }
        NarratorNodeIfc *getBaseNode() const {
            return node;
        }
        bool operator ==(const NodeIterator &rhs) const {
            return (node == rhs.node && i == rhs.i && j == rhs.j &&
                    type == rhs.type); //if we are using list instead of list, no problem also (will be just compared)
        }
        bool isNull() const {
            return operator ==(null);
        }
        bool isChainIterator() const {
            return type == CHAINS;
        }
        static NodeIterator null;
};

#endif
