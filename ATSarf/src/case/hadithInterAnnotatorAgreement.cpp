#include "hadithInterAnnotatorAgreement.h"
#include "hadithDagGraph.h"
#include "hadithChainGraph.h"
#include "Math_functions.h"

HadithInterAnnotatorAgreement::HadithInterAnnotatorAgreement(QString *text, QString fileName,
        AbstractGraph *generatedGraph, OutputDataList &generatedTags, bool displayPORstatistics)
    : AbstractTwoLevelAgreement(text, fileName, generatedGraph, generatedTags) {
#ifdef GRAPH_COMPARE
    nodeCorrect = NULL;
    nodeDetected = NULL;
    countCommon = 0;
    mergesRecallALL = mergesPrecisionALL = mergesRecallNew = mergesPrecisionNew = 0;
    this->displayPORstatistics = displayPORstatistics;
#endif
}

int HadithInterAnnotatorAgreement::equalNames(QString *, int start1, int end1, int start2, int end2) {
    return overLaps(start1, end1, start2, end2);
}

AbstractGraph *HadithInterAnnotatorAgreement::newGraph(bool global) {
    if (global) {
        return new HadithDagGraph();
    } else {
        return new HadithChainGraph();
    }
}

void HadithInterAnnotatorAgreement::displayStatistics() {
    theSarf->displayed_error << "-------------------------\n"
                             << "Segmentation:\n"
                             << "\trecall=\t\t" << commonCount << "/" << correctTagsCount << "=\t" << segmentationRecall << "\n"
                             << "\tprecision=\t\t" << allCommonCount << "/" << generatedTagsCount << "=\t" << segmentationPrecision << "\n"
                             << "\tunder-segmentation=\t" << commonCount << "/" << allCommonCount << "=\t" << underSegmentationRatio << "\n"
                             << "Sanad Boundary (Min-Boundaries):\n"
                             << "\trecall=\t\t" << underNameRecall << "\n"
                             << "\tprecision=\t\t" << underNamePrecision << "\n"
                             << "Sanad Boundary (Max-Boundaries):\n"
                             << "\trecall=\t\t" << nameRecall << "\n"
                             << "\tprecision=\t\t" << namePrecision << "\n"
                             << "Narrator Boundary (Min-boundaries):\n"
                             << "\trecall=\t" << underBoundaryRecall << "\n"
                             << "\tprecision=\t" << underBoundaryPrecision << "\n"
                             << "Narrator Boundary (Max-boundaries):\n"
                             << "\trecall=\t" << boundaryRecall << "\n"
                             << "\tprecision=\t" << boundaryPrecision << "\n";

    if (displayPORstatistics) {
        theSarf->displayed_error << "POR (merged nodes):\n"
                                 << "\tall errors:\n"
                                 << "\t\trecall=\t" << mergesRecallALL << "\n"
                                 << "\t\tprecision=\t" << mergesPrecisionALL << "\n"
                                 << "\tmerging errors:\n"
                                 << "\t\trecall=\t" << mergesRecallNew << "\n"
                                 << "\t\tprecision=\t" << mergesPrecisionNew << "\n";
    }
}

void HadithInterAnnotatorAgreement::initializeAuxiliaryCountersLists() {
#ifdef GRAPH_COMPARE
    chainNodesMapCO.clear();
    chainNodesMapCO.clear();
    nodesMap.clear();
    HadithDagGraph *graphCorrect = dynamic_cast<HadithDagGraph *>(annotatedGraph);
    graphCorrect->getGraph()->fillChainContainer();
    HadithDagGraph *graphGenerated = dynamic_cast<HadithDagGraph *>(generatedGraph);
    graphGenerated->getGraph()->fillChainContainer();
#endif
}

void HadithInterAnnotatorAgreement::startNamesOverLap(int i, int j, int k, int h, int commonCount) {
#ifdef GRAPH_COMPARE
    HadithDagGraph *graphCorrect = dynamic_cast<HadithDagGraph *>(annotatedGraph);
    nodeCorrect = graphCorrect->getGraph()->getChainNode(i, k);
    HadithDagGraph *graphGenerated = dynamic_cast<HadithDagGraph *>(generatedGraph);
    nodeDetected = graphGenerated->getGraph()->getChainNode(j, h);

    if (nodeCorrect == NULL) {
        nodeCorrect = graphCorrect->getGraph()->getChainNode(i, k);
        qDebug() << tags[i].getText();

        for (int f = 0; f < k; f++) {
            nodeCorrect = graphCorrect->getGraph()->getChainNode(i, f);
            qDebug() << nodeCorrect->CanonicalName();
        }
    }

    if (nodeDetected == NULL) {
        nodeDetected = graphGenerated->getGraph()->getChainNode(j, h);
        qDebug() << outputList[j].getText();

        for (int f = 0; f < h; f++) {
            nodeDetected = graphGenerated->getGraph()->getChainNode(j, f);
            qDebug() << nodeDetected->CanonicalName();
        }
    }

    assert(nodeCorrect != NULL && nodeDetected != NULL);
    this->countCommon = commonCount;
#endif
}

void HadithInterAnnotatorAgreement::anotherTagOverLapPreviousOutputName(int /*i*/, int /*j*/, int /*k*/, int /*h*/) {
#ifdef GRAPH_COMPARE
    assert(nodeDetected != NULL && nodeCorrect != NULL);
    ChainNodeCorrespondanceMap::iterator itr = chainNodesMapOC.find(nodeDetected);
    assert(itr != chainNodesMapOC.end());
    ChainNarratorNode *oldCorrect = itr->first;
    int oldSimilarity = itr->second;

    if (oldSimilarity < countCommon) {
        chainNodesMapCO.remove(oldCorrect);
        chainNodesMapOC[nodeDetected] = ChainNodeIntPair(nodeCorrect, countCommon);
        chainNodesMapCO[nodeCorrect] = ChainNodeIntPair(nodeDetected, countCommon);
    }

#endif
}

void HadithInterAnnotatorAgreement::anotherOutputOverLapPreviousTagName(int /*i*/, int /*j*/, int /*k*/, int /*h*/) {
#ifdef GRAPH_COMPARE
    assert(nodeDetected != NULL && nodeCorrect != NULL);
    ChainNodeCorrespondanceMap::iterator itr = chainNodesMapCO.find(nodeCorrect);
    assert(itr != chainNodesMapCO.end());
    ChainNarratorNode *oldDetected = itr->first;
    int oldSimilarity = itr->second;

    if (oldSimilarity < countCommon) {
        chainNodesMapOC.remove(oldDetected);
        chainNodesMapOC[nodeDetected] = ChainNodeIntPair(nodeCorrect, countCommon);
        chainNodesMapCO[nodeCorrect] = ChainNodeIntPair(nodeDetected, countCommon);
    }

#endif
}

void HadithInterAnnotatorAgreement::firstNameOverLap(int /*i*/, int /*j*/, int /*k*/, int /*h*/) {
#ifdef GRAPH_COMPARE
    assert(nodeCorrect != NULL && nodeDetected != NULL);
    chainNodesMapOC[nodeDetected] = ChainNodeIntPair(nodeCorrect, countCommon);
    chainNodesMapCO[nodeCorrect] = ChainNodeIntPair(nodeDetected, countCommon);
#endif
}

void HadithInterAnnotatorAgreement::beforeMovingToNextTagName(int i, int /*j*/, int k, int /*h*/) {
#ifdef GRAPH_COMPARE
    HadithDagGraph *graphCorrect = dynamic_cast<HadithDagGraph *>(annotatedGraph);
    ChainNarratorNode *nodeCorrect = graphCorrect->getGraph()->getChainNode(i, k);
    ChainNodeCorrespondanceMap::iterator itr = chainNodesMapCO.find(nodeCorrect);

    if (nodeCorrect == NULL) {
        qDebug() << tags[i].getText();
    }

    assert(nodeCorrect != NULL);

    if (itr == chainNodesMapCO.end()) {
        chainNodesMapCO[nodeCorrect] = ChainNodeIntPair(NULL, 0);
    }

#endif
}

void HadithInterAnnotatorAgreement::beforeMovingToNextOutputName(int i, int j, int /*k*/, int h) {
#ifdef GRAPH_COMPARE
    HadithDagGraph *graphGenerated = dynamic_cast<HadithDagGraph *>(generatedGraph);
    ChainNarratorNode *nodeDetected = graphGenerated->getGraph()->getChainNode(j, h);
    ChainNodeCorrespondanceMap::iterator itr = chainNodesMapOC.find(nodeDetected);

    if (nodeDetected == NULL) {
        qDebug() << outputList[i].getText();
    }

    assert(nodeDetected != NULL);

    if (itr == chainNodesMapOC.end()) {
        chainNodesMapOC[nodeDetected] = ChainNodeIntPair(NULL, 0);
    }

#endif
}

void HadithInterAnnotatorAgreement::beforeMovingToNextTag(int i, int /*j*/) {
#ifdef GRAPH_COMPARE
    HadithDagGraph *graphCorrect = dynamic_cast<HadithDagGraph *>(annotatedGraph);

    for (int k = 0; true; k++) {
        ChainNarratorNode *nodeCorrect = graphCorrect->getGraph()->getChainNode(i, k);
        ChainNodeCorrespondanceMap::iterator itr = chainNodesMapCO.find(nodeCorrect);

        if (nodeCorrect == NULL) {
            Name n0(text, tags[i - 1].getMainStart(), tags[i - 1].getMainEnd());
            Name n(text, tags[i].getMainStart(), tags[i].getMainEnd());
            qDebug() << n0.getString();
            qDebug() << n.getString();
            qDebug() << tags[i].getText();
        }

        assert(nodeCorrect != NULL);

        if (itr == chainNodesMapCO.end()) {
            chainNodesMapCO[nodeCorrect] = ChainNodeIntPair(NULL, 0);
        }

        if (nodeCorrect->isLast()) {
            break;
        }
    }

#endif
}

void HadithInterAnnotatorAgreement::beforeMovingToNextOutput(int /*i*/, int j) {
#ifdef GRAPH_COMPARE
    HadithDagGraph *graphGenerated = dynamic_cast<HadithDagGraph *>(generatedGraph);

    for (int h = 0; true; h++) {
        ChainNarratorNode *nodeDetected = graphGenerated->getGraph()->getChainNode(j, h);
        ChainNodeCorrespondanceMap::iterator itr = chainNodesMapOC.find(nodeDetected);
        assert(nodeDetected != NULL);

        if (itr == chainNodesMapOC.end()) {
            chainNodesMapOC[nodeDetected] = ChainNodeIntPair(NULL, 0);
        }

        if (nodeDetected->isLast()) {
            break;
        }
    }

#endif
}

void HadithInterAnnotatorAgreement::compareGlobalGraphs(AbstractGraph * /*correctGraph*/,
        AbstractGraph * /*generaredGraph*/) {
    QVector<double> recallList, precisionList, recallAllList, precisionAllList;
    ChainNodeCorrespondanceMap::iterator itr = chainNodesMapCO.begin();
    int includeOldErrorsTotal = 0, newErrorsTotal = 0;

    for (; itr != chainNodesMapCO.end(); ++itr) {
        ChainNarratorNode *correctChainNode = itr.key();
        NarratorNodeIfc *correctNode = &correctChainNode->getCorrespondingNarratorNode();
        int correctMergesCount = getNumChainNodes(correctNode, &chainNodesMapCO) - 1,
            correctAllMergesCount = getNumChainNodes(correctNode) - 1;
        assert(correctMergesCount <= correctAllMergesCount);
        ChainNarratorNode *detectedChainNode = itr->first;

        if (detectedChainNode != NULL) {
            NarratorNodeIfc *detectedNode = &detectedChainNode->getCorrespondingNarratorNode();
            int detectedMergesCount = getNumChainNodes(detectedNode, &chainNodesMapOC) - 1,
                detectedAllMergesCount = getNumChainNodes(detectedNode) - 1;
            assert(detectedMergesCount <= detectedAllMergesCount);
            int commonMergesCount = getCommonNodes(correctNode, detectedNode) - 1;

            //if (detectedAllMergesCount>0 || correctAllMergesCount>0) {
            if (detectedAllMergesCount > 0) {
                double precision = (double)commonMergesCount / detectedAllMergesCount;
                precisionAllList.append(precision);
            } else {
                precisionAllList.append(1);
            }

            if (correctAllMergesCount > 0) {
                double recall = (double)commonMergesCount / correctAllMergesCount;
                recallAllList.append(recall);
            } else {
                recallAllList.append(1);
            }

            if (detectedMergesCount > 0) {
                double precision = (double)commonMergesCount / detectedMergesCount;
                precisionList.append(precision);
            } else {
                precisionList.append(1);
            }

            if (correctMergesCount > 0) {
                double recall = (double)commonMergesCount / correctMergesCount;
                recallList.append(recall);
            } else {
                recallList.append(1);
            }

            newErrorsTotal++;
            /*} else
              continue;*/
        } else { //if we remove this statement we dont support for errors due to unincluded nodes
            recallAllList.append(0);
            precisionAllList.append(0);
        }

        includeOldErrorsTotal++;
    }

    mergesRecallALL = sum(recallAllList) / includeOldErrorsTotal,
    mergesPrecisionALL = sum(precisionAllList) / includeOldErrorsTotal,
    mergesRecallNew = sum(recallList) / newErrorsTotal,
    mergesPrecisionNew = sum(precisionList) / newErrorsTotal;
}

int HadithInterAnnotatorAgreement::getCommonNodes(NarratorNodeIfc *correctNode, NarratorNodeIfc *detectedNode) {
    int count = 0;
    NodeIterator itr = correctNode->begin();

    for (; !itr.isFinished(); ++itr) {
        ChainNarratorNode *correctChainNode = dynamic_cast<ChainNarratorNode *>(itr.getNode());
        ChainNarratorNode *detectedChainNode = chainNodesMapCO[correctChainNode].first;

        if (detectedChainNode != NULL) {
            NarratorNodeIfc *correspondingDetectedNode = &detectedChainNode->getCorrespondingNarratorNode();

            if (correspondingDetectedNode == detectedNode) {
                count++;
            }
        }
    }

    return count;
}

int HadithInterAnnotatorAgreement::getNumChainNodes(NarratorNodeIfc *node, ChainNodeCorrespondanceMap *correspondance) {
    int count = 0;
    NodeIterator itr = node->begin();

    for (; !itr.isFinished(); ++itr) {
        if (correspondance == NULL) {
            count++;
        } else {
            ChainNarratorNode *n = dynamic_cast<ChainNarratorNode *>(itr.getNode());
            ChainNodeCorrespondanceMap::iterator i = correspondance->find(n);
            ChainNarratorNode *corr = i->first;

            if (corr != NULL) {
                count++;
            }
        }
    }

    return count;
}

int hadith_agreement(QString input_str, ATMProgressIFC *prg) {
    QString fileName = input_str.split('\n', QString::SkipEmptyParts)[0];
    QFile input(input_str);

    if (!input.open(QIODevice::ReadOnly)) {
        theSarf->out << "File not found\n";
        return 1;
    }

    QTextStream file(&input);
    file.setCodec("utf-8");
    QString *text = new QString(file.readAll());

    if (text->isNull()) {
        theSarf->out << "file error:" << input.errorString() << "\n";
        return 1;
    }

    if (text->isEmpty()) {//ignore empty files
        theSarf->out << "empty file\n";
        return 0;
    }

    HadithDagGraph *dummyGraph = new HadithDagGraph();
    HadithInterAnnotatorAgreement::OutputDataList dummyList;
    HadithInterAnnotatorAgreement h2(text, fileName, dummyGraph, dummyList);
    QString newFileName = prg->getFileName();
    HadithInterAnnotatorAgreement::OutputDataList tags2;
    HadithDagGraph *graph2 = new HadithDagGraph();
    h2.readAnnotation(newFileName, tags2, graph2);
    HadithInterAnnotatorAgreement h(text, fileName, graph2, tags2, false);
    h.calculateStatisticsOrAnotate();
    h.displayStatistics();
    return 0;
}
