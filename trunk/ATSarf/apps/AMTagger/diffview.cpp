#include "diffview.h"
#include "global.h"
#include "sarftagtype.h"
#include <QGridLayout>
#include <QDockWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QtAlgorithms>

bool compareTags(const Tag &tag1, const Tag &tag2) {
    return (tag1.pos < tag2.pos);
}

bool compareTagTypes(const TagType *tt1, const TagType *tt2) {
    return (tt1->tag < tt2->tag);
}

DiffView::DiffView(QWidget *parent) :
    QMainWindow(parent)
{
    QDockWidget *dock = new QDockWidget(tr("Difference"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    lblCommon = new QLabel(tr("Common"));
    txtCommon = new QTextBrowser();
    QVBoxLayout *vb1 = new QVBoxLayout;
    vb1->setAlignment(Qt::AlignCenter);
    vb1->addWidget(lblCommon);
    vb1->addWidget(txtCommon);
    QScrollArea *sac = new QScrollArea;
    sac->setLayout(vb1);
    lblForwardDiff = new QLabel(tr("A-B"));
    txtForwardDiff = new QTextBrowser();
    QVBoxLayout *vb2 = new QVBoxLayout;
    vb2->setAlignment(Qt::AlignCenter);
    vb2->addWidget(lblForwardDiff);
    vb2->addWidget(txtForwardDiff);
    QScrollArea *saf = new QScrollArea;
    saf->setLayout(vb2);
    lblReverseDiff = new QLabel(tr("B-A"));
    txtReverseDiff = new QTextBrowser();
    QVBoxLayout *vb3 = new QVBoxLayout;
    vb3->setAlignment(Qt::AlignCenter);
    vb3->addWidget(lblReverseDiff);
    vb3->addWidget(txtReverseDiff);
    QScrollArea *sar = new QScrollArea;
    sar->setLayout(vb3);
    QSplitter *splitter = new QSplitter(dock);
    splitter->addWidget(sac);
    splitter->addWidget(saf);
    splitter->addWidget(sar);
    dock->setWidget(splitter);
    addDockWidget(Qt::TopDockWidgetArea, dock);

    dock = new QDockWidget(tr("Options"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    QHBoxLayout *vboxmain = new QHBoxLayout;
    gb1 = new QGroupBox(tr("Difference"));
    rbTTDiff = new QRadioButton(tr("TagType"));
    connect(rbTTDiff,SIGNAL(clicked()),this,SLOT(rbTagTypes_clicked()));
    rbTDiff = new QRadioButton(tr("Tag"));
    connect(rbTDiff,SIGNAL(clicked()),this,SLOT(rbTags_clicked()));
    rbTTDiff->setChecked(true);
    QVBoxLayout *vbox1 = new QVBoxLayout;
    vbox1->addWidget(rbTTDiff);
    vbox1->addWidget(rbTDiff);
    gb1->setLayout(vbox1);
    vboxmain->addWidget(gb1);
    gb2 = new QGroupBox(tr("Criteria"));
    rbExact = new QRadioButton(tr("Exact"));
    connect(rbExact,SIGNAL(clicked()),this,SLOT(rbExact_clicked()));
    rbIntersect = new QRadioButton(tr("Intersect"));
    connect(rbIntersect,SIGNAL(clicked()),this,SLOT(rbIntersect_clicked()));
    rbAContainB = new QRadioButton(tr("A include B"));
    connect(rbAContainB,SIGNAL(clicked()),this,SLOT(rbAContainB_clicked()));
    rbBContainA = new QRadioButton(tr("B include A"));
    connect(rbBContainA,SIGNAL(clicked()),this,SLOT(rbBContainA_clicked()));
    rbIntersect->setChecked(true);
    gb2->setEnabled(false);
    QVBoxLayout *vbox2 = new QVBoxLayout;
    vbox2->addWidget(rbIntersect);
    vbox2->addWidget(rbExact);
    vbox2->addWidget(rbAContainB);
    vbox2->addWidget(rbBContainA);
    gb2->setLayout(vbox2);
    vboxmain->addWidget(gb2);
    QScrollArea *sa1 = new QScrollArea(dock);
    sa1->setLayout(vboxmain);
    sa1->setMaximumWidth(250);
    dock->setWidget(sa1);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock = new QDockWidget(tr("Statistics"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    txtStats = new QTextBrowser(dock);
    dock->setWidget(txtStats);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    setWindowTitle(tr("Difference Analysis Statistics"));
    resize(800,600);

    /** Sort the Tags in both Tag Vectors **/
    qSort(_atagger->tagVector.begin(), _atagger->tagVector.end(), compareTags);
    qSort(_atagger->compareToTagVector.begin(), _atagger->compareToTagVector.end(), compareTags);

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->tag == ctt->tag) {
                found = true;
                commonTT.append(stt->tag);
            }
        }
        if(!found) {
            forwardTT.append(stt->tag);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->tag))){
            reverseTT.append(ctt->tag);
        }
    }

    QString text;
    for(int i=0; i<commonTT.count(); i++) {
        text += commonTT[i] + '\n';
    }
    txtCommon->setText(text);

    text.clear();
    for(int i=0; i<forwardTT.count(); i++) {
        text += forwardTT[i] + '\n';
    }
    txtForwardDiff->setText(text);

    text.clear();
    for(int i=0; i<reverseTT.count(); i++) {
        text += reverseTT[i] + '\n';
    }
    txtReverseDiff->setText(text);
}

void DiffView::rbTagTypes_clicked() {
    gb2->setEnabled(false);
    rbIntersect->setChecked(true);

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze tagtype difference **/

    for(int i=0; i<_atagger->tagTypeVector->count(); i++) {
        const TagType * stt = (TagType*)(_atagger->tagTypeVector->at(i));
        bool found = false;
        for(int j=0; j<_atagger->compareToTagTypeVector->count();j++) {
            const TagType * ctt = (TagType*)(_atagger->compareToTagTypeVector->at(j));
            if(stt->tag == ctt->tag) {
                found = true;
                commonTT.append(stt->tag);
            }
        }
        if(!found) {
            forwardTT.append(stt->tag);
        }
    }

    for(int i=0; i<_atagger->compareToTagTypeVector->count();i++) {
        const TagType * ctt = (TagType*)(_atagger->tagTypeVector->at(i));
        if(!(commonTT.contains(ctt->tag))){
            reverseTT.append(ctt->tag);
        }
    }

    QString text;
    for(int i=0; i<commonTT.count(); i++) {
        text += commonTT[i] + '\n';
    }
    txtCommon->setText(text);

    text.clear();
    for(int i=0; i<forwardTT.count(); i++) {
        text += forwardTT[i] + '\n';
    }
    txtForwardDiff->setText(text);

    text.clear();
    for(int i=0; i<reverseTT.count(); i++) {
        text += reverseTT[i] + '\n';
    }
    txtReverseDiff->setText(text);
}

void DiffView::rbTags_clicked() {
    gb2->setEnabled(true);

    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags Difference **/


}

void DiffView::rbExact_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();

    /** Analyze Tags based on Exact Match **/

    QVector<Tag*> common;
    QVector<Tag*> forward;
    QVector<Tag*> reverse;

    // apply an algorithm similar to mergesort
}

void DiffView::rbIntersect_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();
}

void DiffView::rbAContainB_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();
}

void DiffView::rbBContainA_clicked() {
    txtCommon->clear();
    txtForwardDiff->clear();
    txtReverseDiff->clear();
    txtStats->clear();
}
