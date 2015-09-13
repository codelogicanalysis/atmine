#ifndef BIBLEGENEOLOGY_H
#define BIBLEGENEOLOGY_H

#include <QString>
#include <QDebug>
#include <QFile>
#include "common.h"
#include "textParsing.h"
#include "text_handling.h"
#include "ATMProgressIFC.h"
#include "abstractGraph.h"


class GeneologyParameters {
    public:
        unsigned int theta_0: 14;
        unsigned int L_min: 9;
        unsigned int N_min: 9;
        unsigned int C_max: 10;
        unsigned int radius: 10;
        unsigned int step: 10;
        bool detailed_statistics: 1;
        bool refined: 1;
        GeneologyParameters() :
            theta_0(45),
            L_min(3),
            N_min(3),
            C_max(2),
            radius(3),
            step(10),
            detailed_statistics(true),
            refined(false) {}
};

const static QString abram   = QString("") + alef_hamza_above + ba2 + ra2 + alef + meem;
const static QString ibrahim = QString("") + alef_hamza_below + ba2 + ra2 + alef + ha2 + ya2 + meem;
const static QString sarah   = QString("") + seen + alef + ra2 + ta2_marbouta;
const static QString saray   = QString("") + seen + alef + ra2 + alef + ya2;
const static QString ya3coub = QString("") + ya2 + _3yn + qaf + waw + ba2;
const static QString israel  = QString("") + alef_hamza_below + seen + ra2 + alef + ya2_hamza_above + ya2 + lam;

inline bool equalNames(const QString &n1, const QString &n2) {
    if (equal_withoutLastDiacritics(n1, abram) && equal_withoutLastDiacritics(n2, ibrahim)) {
        return true;
    }

    if (equal_withoutLastDiacritics(n2, abram) && equal_withoutLastDiacritics(n1, ibrahim)) {
        return true;
    }

    if (equal_withoutLastDiacritics(n1, sarah) && equal_withoutLastDiacritics(n2, saray)) {
        return true;
    }

    if (equal_withoutLastDiacritics(n2, saray) && equal_withoutLastDiacritics(n1, sarah)) {
        return true;
    }

    if (equal_withoutLastDiacritics(n1, ya3coub) && equal_withoutLastDiacritics(n2, israel)) {
        return true;
    }

    if (equal_withoutLastDiacritics(n2, israel) && equal_withoutLastDiacritics(n1, ya3coub)) {
        return true;
    }

    QString s1 = withoutAL(n1), s2 = withoutAL(n2);
    assert(!equal_withoutLastDiacritics(n1, n2) || equal_withoutLastDiacritics(s1, s2));
    return equal_withoutLastDiacritics(s1, s2);
}

extern GeneologyParameters geneologyParameters;

int genealogyHelper(QString input_str, ATMProgressIFC *prgs);
void geneology_initialize();

class GeneNode;
class FillTextVisitor;

class Name {
    private:
        friend class FillTextVisitor;
        friend class FixSpousesVisitor;
        friend class AbstractGeneNode;
        friend class GeneNode;
        friend class SpouseGeneNode;
        friend QDataStream &operator>>(QDataStream &in, Name &t);
        friend QDataStream &operator<<(QDataStream &out, const Name &t);

        long start, end;
        QString *text;
        bool male: 1;
    private:
        friend QDataStream &operator>>(QDataStream &in, GeneNode &t);
        Name() {}
    public:
        Name(QString *text, long start, long end, bool male = true) {
            this->start = start;
            this->end = end;
            this->text = text;
            this->male = male;
        }
        Name &operator=(const Name &n2) {
            text = n2.text;
            start = n2.start;
            end = n2.end;
            male = n2.male;
            return *this;
        }
        bool isMarriageCompatible(const Name &n) const {
            return (n.male != male);
        }
        bool operator ==(const Name &n) const {
            return equalNames(getString(), n.getString());
        }
        QString getString() const {
            return text->mid(start, end - start + 1);
        }
        QString *getTextPointer() const {
            return text;
        }
        long getStart() const {
            return start;
        }
        long getLength() const {
            return end - start + 1;
        }
        long getEnd() const {
            return end;
        }
        bool operator<(const Name &n) const {
            return getString() < n.getString();
        }
        bool isMale() const {
            return male;
        }
};

inline int qHash(const Name &n) {
    if (equal_withoutLastDiacritics(n.getString(), abram)) {
        return qHash(ibrahim);
    }

    if (equal_withoutLastDiacritics(n.getString(), saray)) {
        return qHash(sarah);
    }

    if (equal_withoutLastDiacritics(n.getString(), israel)) {
        return qHash(ya3coub);
    }

    return qHash(withoutAL(removeDiacritics(n.getString())));
}


class GeneNode;
class GeneTree;

class AbstractGeneNode {
        friend class FillTextVisitor;
        friend class GeneTree;
        friend class BibleTaggerDialog;
        friend QDataStream &operator<<(QDataStream &out, const GeneNode &t);

    protected:
        Name name;
        GeneNode *parent;
        QList<QString> edgeText;

        AbstractGeneNode() : name() {
            parent = NULL;
        }

    public:
        AbstractGeneNode(const Name &n, GeneNode *parent): name(n) {
            assert((void *)parent != (void *)this);
            this->parent = parent;
        }

        virtual ~AbstractGeneNode() {}

        virtual bool isSpouse() const {
            return false;
        }
        GeneNode *getParent() {
            return parent;
        }
        void setEdgeText(QString edge) {
            edgeText.clear();
            edgeText.append(edge);
        }
        void copyEdgeText(AbstractGeneNode *node) {
            if (node == NULL) {
                return;
            }

            edgeText = node->edgeText;
        }
        void addEdgeText(QString edge) {
            edgeText.append(edge);
        }
        QString getEdgeText() const {
#define MAX_SIZE 20
            QString edge;

            for (int i = 0; i < edgeText.size(); i++) {
                edge += (i == 0 ? "" : "/") + edgeText[i].left(MAX_SIZE);

                if (edgeText[i].size() > MAX_SIZE) {
                    edge += "...";
                }
            }

            return edge;
#undef MAX_SIZE
        }
        QString getFullEdgeText() const {
            QString edge;

            for (int i = 0; i < edgeText.size(); i++) {
                edge += (i == 0 ? "" : "/") + edgeText[i];
            }

            return edge;
        }
        const Name &getName() const {
            assert(this != NULL);
            return name;
        }
        QString getGender() const {
            return (name.isMale() ? "M" : "F");
        }
        QString getString() const {
            return name.getString();
        }
        virtual int getGraphHeight() const = 0;
        virtual int getDirectGraphChildrenCount() const {
            return 0;
        }
        virtual int getDescendentCount() const {
            return 0;
        }
};
class SpouseGeneNode: public AbstractGeneNode {
        friend QDataStream &operator>>(QDataStream &in, GeneNode &t);
        friend QDataStream &operator>>(QDataStream &in, GeneTree &t);
    protected:
        friend class GeneNode;
        SpouseGeneNode() {}

    public:
        SpouseGeneNode(const Name &n, GeneNode *parent): AbstractGeneNode(n, parent) {}
        virtual ~SpouseGeneNode() {}
        virtual bool isSpouse() const {
            return true;
        }
        virtual int getGraphHeight() const;
};

class GeneNode: public AbstractGeneNode {
    private:
        friend QDataStream &operator>>(QDataStream &in, GeneNode &t);
        friend QDataStream &operator>>(QDataStream &in, GeneTree &t);
        friend class GeneTree;
        void setParentHeight(int height) {
            if (this == parent) {
                qDebug() << parent->toString();
                return;
            }

            if (parent != NULL) {
                parent->height = max(parent->height, height);
                parent->setParentHeight(parent->height + 1);
            }
        }
        void deleteSubTree() {
            for (int i = 0; i < children.size(); i++) {
                children[i]->deleteSubTree();
            }

            for (int i = 0; i < spouses.size(); i++) {
                delete spouses[i];
            }

            delete this;
        }
        GeneNode *getNodeInSubTree(QString word, bool checkSpouses = false, int maxDepth = -1) {
            AbstractGeneNode *node = getAbstractNodeInSubTree(word, checkSpouses, maxDepth);

            if (node == NULL) {
                return NULL;
            }

            if (node->isSpouse()) {
                return node->getParent();
            } else {
                return dynamic_cast<GeneNode *>(node);
            }
        }
        AbstractGeneNode *getAbstractNodeInSubTree(QString word, bool checkSpouses = false, int maxDepth = -1) {
            if (!ignoreInSearch && equalNames(word, name.getString())) {
                return this;
            }

            if (checkSpouses) {
                for (int i = 0; i < spouses.size(); i++) {
                    if (equalNames(word, spouses[i]->getString())) {
                        return spouses[i];
                    }
                }
            }

            if (maxDepth < 0 || maxDepth > 0) {
                int new_maxDepth = (maxDepth < 0 ? maxDepth : maxDepth - 1);

                for (int i = 0; i < children.size(); i++) {
                    GeneNode *found = children[i]->getNodeInSubTree(word, checkSpouses, new_maxDepth);

                    if (found != NULL) {
                        return found;
                    }
                }
            }

            return NULL;
        }
        void printTree(int indentation) {
            QString sp = "";

            if (spouses.size() > 0) {
                sp += "[";

                for (int i = 0; i < spouses.size(); i++) {
                    sp += (i != 0 ? "\t" : "") + spouses[i]->getString();
                }

                sp += "] ";
            }

            theSarf->out << QString(indentation, '>')/*<<"["<<height<<"]"*/ << name.getString() << sp << "\n";

            for (int i = 0; i < children.size(); i++) {
                children[i]->printTree(indentation + 1);
            }
        }

        GeneNode(): AbstractGeneNode() {}

    public:
        QList<SpouseGeneNode *> spouses;
        QList<GeneNode *> children;
        int height;
        bool ignoreInSearch;

        GeneNode(const Name &n, GeneNode *parent): AbstractGeneNode(n, parent) {
            height = 0; //since no children
            setParentHeight(1);

            if (parent != NULL) {
                parent->addChild(this);
            }

            ignoreInSearch = false;
        }

        virtual ~GeneNode() {}

        GeneNode *addChild(GeneNode *n) {   //return itself
            if (n != NULL) {
                children.append(n);
                n->parent = this;
                n->setParentHeight(n->height + 1);
            }

            return this;
        }
        GeneNode *addParent(GeneNode *n) {   //return new parent
            if (n != NULL) {
                if (parent != NULL) {
                    parent->addSpouse(n->name);

                    //n->name.parent=parent;
                    for (int i = 0; i < n->spouses.size(); i++) {
                        parent->addSpouse(n->spouses[i]);
                    }

                    /*for (int i=0;i<parent->spouses.size();i++) {
                      parent->spouses[i].parent=parent;
                      }*/
                    return parent;
                } else {
                    n->addChild(this);
                    //name.parent=n;
                    return n;
                }
            }

            return this;
        }
        bool hasSpouse(const Name &n, bool checkWithMain = false) const {
            if (checkWithMain && equalNames(n.getString(), name.getString())) {
                return true;
            }

            for (int i = 0; i < spouses.size(); i++) {
                if (equalNames(n.getString(), spouses[i]->getString())) {
                    return true;
                }
            }

            return false;
        }
        bool hasChild(const Name &n) const {
            for (int i = 0; i < children.size(); i++) {
                if (equalNames(n.getString(), children[i]->toString())) {
                    return true;
                }
            }

            return false;
        }
        GeneNode *addSpouse(const Name &n) {   //return itself
            SpouseGeneNode *spouse = new SpouseGeneNode(n, this);
            spouses.append(spouse);
            return this;
        }
        GeneNode *addSpouse(SpouseGeneNode *spouse) {   //return itself
            spouse->parent = this;
            spouses.append(new SpouseGeneNode(*spouse));
            return this;
        }
        GeneNode *addSibling(GeneNode *n) {   //return itself
            if (parent != NULL) {
                parent->children.append(n);

                if (n != NULL) {
                    n->parent = parent;
                    parent->setParentHeight(n->height + 1);
                }
            }

            return this;
        }
        bool isLeaf() {
            return children.size() == 0;
        }
        int getSubTreeCount(bool countSpouses = false) const {
            return getSubTreeCount(NULL, countSpouses);
        }
        int getSubTreeCount(QSet<Name> *visited, bool countSpouses = false) const  {
            int count = 0;

            if (visited == NULL) {
                count = 1;
            } else if (!visited->contains(name)) {
                visited->insert(name);
                count = 1;
            }

            for (int i = 0; i < children.size(); i++) {
                count += children[i]->getSubTreeCount(visited, countSpouses);
            }

            if (countSpouses) {
                count += spouses.size();
            }

            return count;
        }
        virtual int getGraphHeight() const {
            return height;
        }
        virtual int getDirectGraphChildrenCount() const {
            return children.size() + spouses.size();
        }
        virtual int getDescendentCount()  const {
            return getSubTreeCount(true) - 1;
        }
        QString toString() const {
            return getString();
        }
};

class GeneTree: public AbstractGraph {
    public:
        class MergeVisitor;
        class GraphStatistics {
            public:
                double foundRecall, foundPrecision,
                       contextRecall, contextPrecision,
                       neigborhoodRecall, neigborhoodPrecision,
                       spousesRecall, spousesPrecision,
                       childrenRecall, childrenPrecision;

                GraphStatistics() {
                    foundRecall = 0;
                    foundPrecision = 0;
                    contextRecall = 0;
                    contextPrecision = 0;
                    neigborhoodRecall = 0;
                    neigborhoodPrecision = 0;
                    spousesRecall = 0;
                    spousesPrecision = 0;
                    childrenRecall = 0;
                    childrenPrecision = 0;
                }
        };
    private:
        friend QDataStream &operator<<(QDataStream &out, const GeneTree &t);
        friend QDataStream &operator>>(QDataStream &in, GeneTree &t);
        MergeVisitor *mergeVisitor;
        GeneNode *root;
    private:
        void init(GeneNode *root);
        virtual AbstractGraph *readFromStreamHelper(QDataStream &in);
    public:
        explicit GeneTree() : AbstractGraph() {
            root = NULL;
            mergeVisitor = NULL;
        }
        explicit GeneTree(GeneNode *root) : AbstractGraph() {
            this->root = root;
            mergeVisitor = NULL;
        }
        virtual ~GeneTree();

        void setRoot(GeneNode *node) {
            root = node;
        }
        GeneNode *getRoot() {
            return root;
        }
        // TODO: Is suicide safe here?
        void deleteGraph() {
            //out<<"\n{deleting}\n";
            //root->printTree(0);
            //root->deleteSubTree();
            delete this;
        }
        void updateRoot() {
            if (root == NULL) {
                return;
            }

            while (root->parent != NULL) {
                root = root->parent;
            }
        }
        void outputTree() {
            //out<<"{Output}\n";
            root->printTree(0);
            theSarf->out << "\n";
        }
        int getTreeLevels() {
            if (root == NULL) {
                return 0;
            }

            return root->height + 1;
        }
        int getTreeNodesCount(bool countSpouses = false) {
            if (root == NULL) {
                return 0;
            }
            return root->getSubTreeCount(countSpouses);
        }
        int getTreeDistinctNodesCount(bool countSpouses = false) {
            if (root == NULL) {
                return 0;
            }
            QSet<Name> visited;
            return root->getSubTreeCount(&visited, countSpouses);
        }
        GeneNode *findTreeNode(QString word, bool checkSpouses = false) {
            if (root == NULL) {
                return NULL;
            }
            return root->getNodeInSubTree(word, checkSpouses);
        }
        AbstractGeneNode *findAbstractTreeNode(QString word, bool checkSpouses = false) {
            if (root == NULL) {
                return NULL;
            }
            return root->getAbstractNodeInSubTree(word, checkSpouses);
        }
        void compareToStandardTree(GeneTree *standard, GraphStatistics &stats);
        void compareToStandardTree(GeneTree *standard, QSet<QPair<GeneNode *, Name> > &visitedNodes, GraphStatistics &stats);
        AbstractGraph *merge(AbstractGraph *tree);
        void merge(AbstractGraph *tree, QList<int> &delimetersStart, QList<int> &delimetersEnd);
        void mergeLeftovers();
        void displayGraph(ATMProgressIFC *prg);
        virtual void fillNullGraph(MainSelectionList &names, QString *text);
        void fillNullGraph(Name &name);
        virtual QString getText();
        virtual bool isRepresentativeOf(const MainSelectionList &list);
        virtual void fillTextPointers(QString *text);
        virtual QAbstractItemModel *getTreeModel();
        virtual bool buildFromText(QString text, TwoLevelSelection *sel, QString *fileText, AbstractTwoLevelAnnotator *tagger);
        virtual void removeNameFromGraph(Name &name);
        virtual void addNameToGraph(Name &name);
        virtual void writeToStream(QDataStream &out);
        virtual GeneTree *duplicate();
        virtual GeneTree *duplicate(QList<int> &delimitersStart, QList<int> &delimitersEnd);
        virtual bool isEmpty() const {
            return root == NULL;
        }
};

class GeneVisitor {
    private:
        void visit(GeneNode *node) {
            if (node == NULL) {
                return;
            }

            visit(node, node->height);

            for (int i = 0; i < node->spouses.size(); i++) {
                visit(node, node->spouses[i], true);
            }

            for (int i = 0; i < node->children.size(); i++) {
                if (node->children[i] != NULL) {
                    visit(node, node->children[i], false);
                }
            }

            for (int i = 0; i < node->children.size(); i++) {
                visit(node->children[i]);
            }
        }
    public:
        virtual void visit(GeneNode *node, int height) = 0;
        virtual void visit(GeneNode *node1, AbstractGeneNode *node2, bool isSpouse) = 0;
        virtual void finish() = 0;
        void operator()(GeneTree *tree) {
            GeneNode *root = tree->getRoot();
            //tree->updateRoot();
            visit(root);
            finish();
        }
};
class FillTextVisitor: public GeneVisitor {
    private:
        QString *text;
    public:
        explicit FillTextVisitor(QString *text) {
            this->text = text;
        }
        void visit(GeneNode *node, int) {
            node->name.text = text;
        }
        void visit(GeneNode *, AbstractGeneNode *node2, bool isSpouse) {
            if (isSpouse) {
                node2->name.text = text;
            }
        }
        void finish() {}
};


inline QDataStream &operator<<(QDataStream &out, const Name &t) {
    out << (qint32)t.start << (qint32)t.end;
    return out;
}
inline QDataStream &operator>>(QDataStream &in, Name &t) {
    qint32 l, e;
    in >> l >> e;
    t.start = l;
    t.end = e;
    t.text = NULL;
    return in;
}
inline QDataStream &operator<<(QDataStream &out, const GeneNode &t) {
    out << false
        << t.name
        << t.height
        << t.ignoreInSearch
        << t.children.size();

    for (int i = 0; i < t.children.size(); i++) {
        out << *(t.children[i]);
    }

    out << t.spouses.size();

    for (int i = 0; i < t.spouses.size(); i++) {
        out << t.spouses[i]->getName();
    }

    return out;
}

inline QDataStream &operator>>(QDataStream &in, GeneNode &t) {
    int childrenCount = 0;
    int spouseCount = 0;
    in >> t.name
       >> t.height
       >> t.ignoreInSearch
       >> childrenCount;
    t.parent = NULL;

    for (int i = 0; i < childrenCount; i++) {
        bool isNull = false;
        in >> isNull;
        assert(!isNull);
        GeneNode *n = new GeneNode();
        in >> *n;
        t.addChild(n);
    }

    in  >> spouseCount;

    for (int i = 0; i < spouseCount; i++) {
        SpouseGeneNode *s = new SpouseGeneNode();
        in >> s->name;
        t.addSpouse(s);
    }

    return in;
}

inline QDataStream &operator<<(QDataStream &out, const GeneTree &t) {
    out << *t.root;
    return out;
}
inline QDataStream &operator>>(QDataStream &in, GeneTree &t) {
    bool isNull = false;
    in >> isNull;

    if (!isNull) {
        t.root = new GeneNode();
        in >> *t.root;
        t.root->parent = NULL;
    } else {
        t.root = NULL;
    }

    return in;
}

#endif // BIBLEGENEOLOGY_H
