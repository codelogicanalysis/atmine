#include <QMessageBox>
#include "hadithDagGraph.h"
#include "twoLevelTaggerSelection.h"
#include "hadithChainItemModel.h"
#include "bibleGeneology.h"
#include "hadithChainGraph.h"
#include "hadithDagItemModel.h"

HadithDagGraph::HadithDagGraph() {
    ChainsContainer c;
    graph = new NarratorGraph(c, &dummyATMProgressIFC);
}

HadithDagGraph::HadithDagGraph(ChainsContainer &c) {
    graph = new NarratorGraph(c, &dummyATMProgressIFC);
}

HadithDagGraph::HadithDagGraph(NarratorGraph *graph) {
    this->graph = graph;
}

AbstractGraph *HadithDagGraph::readFromStreamHelper(QDataStream &in) {
    graph = new NarratorGraph(in, &dummyATMProgressIFC);
    return this;
}
void HadithDagGraph::writeToStream(QDataStream &out) {
    graph->serialize(out);
}

AbstractGraph *HadithDagGraph::duplicate() {
    return this;
}
AbstractGraph *HadithDagGraph::merge(AbstractGraph *graph2) {
    HadithChainGraph *chainGraph = dynamic_cast<HadithChainGraph *>(graph2); //otherwise, merge is not supported
    ChainsContainer chains;
    Chain *chain = new Chain(chainGraph->chain);
    chains.append(chain);
    HadithDagGraph *c = new HadithDagGraph(chains);
    graph->mergeWith(c->graph);
    qDebug() << graph->hash.size();
    c->graph = NULL;
    delete c;
    chainGraph->chain.m_chain.clear();
    return this;
}
bool HadithDagGraph::isRepresentativeOf(const MainSelectionList &/*list*/) {
    return true; //most probably not needed for global graph
}
QString HadithDagGraph::getText() {
    QString text;

    for (int i = 0; i < graph->all_nodes.size(); i++) {
        NarratorNodeIfc *node = graph->all_nodes[i];

        if (node == NULL || !node->isActualNode()) {
            continue;
        }

        if (node->isGraphNode()) {
            GraphNarratorNode *graphNode = dynamic_cast<GraphNarratorNode *>(node);
            text += graphNode->CanonicalName() + "\n";
            //iterator
            NodeIterator itr = graphNode->begin();

            for (; !itr.isFinished(); ++itr) {
                text += ">" + itr.getNode()->CanonicalName() + "\n";
            }

            text += "\n";
        } else {
            assert(node->isChainNode());
            ChainNarratorNode *chainNode = dynamic_cast<ChainNarratorNode *>(node);
            QString name = chainNode->CanonicalName();
            text += name + "\n\n";
            //text+=">"+name+"\n";
        }
    }

    return text;
}
bool HadithDagGraph::buildFromText(QString /*text*/, TwoLevelSelection */*sel*/, QString */*string*/,
                                   AbstractTwoLevelAnnotator */*annotator*/) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Error Processing Graph");
    msgBox.setText("Not supported for POR, use instead the menus of the Tree View");
    msgBox.exec();
    return false;
}

QAbstractItemModel *HadithDagGraph::getTreeModel() {
    return new HadithDagItemModel(this);
}

void HadithDagGraph::displayGraph(ATMProgressIFC *prg) {
    QIODevice *oldDevice = theSarf->out.device();
    QString s;
    theSarf->out.setString(&s);
    DisplayNodeVisitor visitor;
    GraphVisitorController c(&visitor, graph, true, true);
    graph->DFS_traverse(c);
    theSarf->out.setDevice(oldDevice);
    prg->displayGraph(this);
}

void HadithDagGraph::deleteGraph() {
    delete graph;
}

void HadithDagGraph::fillTextPointers(QString *text) {
    for (int i = 0; i < graph->all_nodes.size(); i++) {
        NarratorNodeIfc *node = graph->all_nodes[i];

        if (node == NULL) {
            continue;
        }

        if (node->isChainNode()) {
            ChainNarratorNode *cNode = dynamic_cast<ChainNarratorNode *>(node);
            cNode->getNarrator().hadith_text = text;

            for (int j = 0; j < cNode->getNarrator().m_narrator.size(); j++) {
                cNode->getNarrator().m_narrator[j]->hadith_text = text;
            }
        }
    }
}

void HadithDagGraph::fillNullGraph(MainSelectionList &/*names*/, QString */*text*/) {
    //I dont thinkn it is needed in our case
}

void HadithDagGraph::addNameToGraph(Name &/*name*/) {
    //not supported
}

void HadithDagGraph::removeNameFromGraph(Name &/*name*/) {
    //not supported
}
