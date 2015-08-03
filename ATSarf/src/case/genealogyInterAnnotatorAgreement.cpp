#include "genealogyInterAnnotatorAgreement.h"
#include "bibleGeneology.h"

GenealogyInterAnnotatorAgreement::GenealogyInterAnnotatorAgreement(QString *text, QString fileName,
        AbstractGraph *generatedGraph, OutputDataList &generatedTags)
    : AbstractTwoLevelAgreement(text, fileName, generatedGraph, generatedTags) { }

int GenealogyInterAnnotatorAgreement::equalNames(QString *text , int start1, int end1, int start2, int end2) {
    Name n1(text, start1, end1);
    QString s1 = n1.getString();
    Name n2(text, start2, end2);
    QString s2 = n2.getString();
    return ::equalNames(s1, s2);
}

AbstractGraph *GenealogyInterAnnotatorAgreement::newGraph(bool) {
    return new GeneTree();
}

void GenealogyInterAnnotatorAgreement::displayStatistics() {
    theSarf->displayed_error << "-------------------------\n"
                             << "Segmentation:\n"
                             << "\trecall=\t\t" << commonCount << "/" << correctTagsCount << "=\t" << segmentationRecall << "\n"
                             << "\tprecision=\t\t" << allCommonCount << "/" << generatedTagsCount << "=\t" << segmentationPrecision << "\n"
                             << "\tunder-segmentation=\t" << commonCount << "/" << allCommonCount << "=\t" << underSegmentationRatio << "\n"
                             << "Boundary (Min-Boundaries):\n"
                             << "\trecall=\t\t" << underNameRecall << "\n"
                             << "\tprecision=\t\t" << underNamePrecision << "\n"
                             << "Boundary (Max-Boundaries):\n"
                             << "\trecall=\t\t" << nameRecall << "\n"
                             << "\tprecision=\t\t" << namePrecision << "\n";
}


int genealogy_agreement(QString input_str, ATMProgressIFC *prg) {
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

    GeneTree *dummyGraph = new GeneTree();
    GenealogyInterAnnotatorAgreement::OutputDataList dummyList;
    GenealogyInterAnnotatorAgreement h2(text, fileName, dummyGraph, dummyList);
    QString newFileName = prg->getFileName();
    GenealogyInterAnnotatorAgreement::OutputDataList tags2;
    GeneTree *graph2 = new GeneTree();
    h2.readAnnotation(newFileName, tags2, graph2);
    GenealogyInterAnnotatorAgreement h(text, fileName, graph2, tags2);
    h.calculateStatisticsOrAnotate();
    h.displayStatistics();
    return 0;
}
