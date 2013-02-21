#include "diffview.h"
#include "global.h"
#include "sarftagtype.h"
#include <QGridLayout>
#include <QDockWidget>
#include <QScrollArea>
#include <QSplitter>

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
    QGroupBox *gb1 = new QGroupBox(tr("Difference"));
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
    QGroupBox *gb2 = new QGroupBox(tr("Criteria"));
    rbExact = new QRadioButton(tr("Exact"));
    connect(rbExact,SIGNAL(clicked()),this,SLOT(rbExact_clicked()));
    rbIntersect = new QRadioButton(tr("Intersect"));
    connect(rbIntersect,SIGNAL(clicked()),this,SLOT(rbIntersect_clicked()));
    rbAContainB = new QRadioButton(tr("A include B"));
    connect(rbAContainB,SIGNAL(clicked()),this,SLOT(rbAContainB_clicked()));
    rbBContainA = new QRadioButton(tr("B include A"));
    connect(rbBContainA,SIGNAL(clicked()),this,SLOT(rbBContainA_clicked()));
    rbIntersect->setChecked(true);
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
}

void DiffView::rbTagTypes_clicked() {

}

void DiffView::rbTags_clicked() {

}

void DiffView::rbExact_clicked() {

}

void DiffView::rbIntersect_clicked() {

}

void DiffView::rbAContainB_clicked() {

}

void DiffView::rbBContainA_clicked() {

}
