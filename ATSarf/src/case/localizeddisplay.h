#ifndef LOCALIZEDDISPLAY_H
#define LOCALIZEDDISPLAY_H

#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QHeaderView>
#include <QStringList>
#include <QComboBox>
#include <QTableWidget>
#include <QTextBrowser>
#include <QInputDialog>

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>

#include "sql_queries.h"
#include "logger.h"
#include "ATMProgressIFC.h"
#include "graph.h"
#include <iostream>

using namespace std;

class LocalizedDisplay: public QMainWindow, public ATMProgressIFC {
        Q_OBJECT
    public:
        LocalizedDisplay(NarratorGraph *graph): QMainWindow() {
            this->graph = graph;
            input_label = new QLabel("Narrator:", this);
            input = new QTextEdit(this);
            input->setMaximumHeight(30);
            find = new QPushButton("&Find Narrator", this);
            display = new QPushButton("Display Neighborhood", this);
            biography = new QPushButton("Find Biography", this);
            steps_label = new QLabel("Steps", this);
            steps = new QTextEdit("1", this);
            steps->setAlignment(Qt::AlignCenter);
            steps->setMaximumHeight(30);
            steps->setMaximumWidth(60);
            narratorListDisplay = new QTableWidget(0, 2, this);
            narratorListDisplay->clear();
            QStringList v;
            v << "Narrator" << "Similarity";
            narratorListDisplay->verticalHeader()->setHidden(true);
            narratorListDisplay->setHorizontalHeaderLabels(v);
            narratorListDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
            narratorListDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
            narratorListDisplay->setSelectionMode(QAbstractItemView::SingleSelection);
            scrollArea = new QScrollArea(this);
            subScrollArea = new QScrollArea(scrollArea);
            pic = new QLabel(subScrollArea);
            subScrollArea->setWidget(pic);
            progressBar = new QProgressBar(this);
            grid = new QGridLayout(scrollArea);
            grid->addWidget(input_label, 0, 0);
            grid->addWidget(input, 0, 1, 1, 5);
            grid->addWidget(find, 0, 6);
            grid->addWidget(steps_label, 0, 7);
            grid->addWidget(steps, 0, 8);
            grid->addWidget(display, 0, 9);
            grid->addWidget(biography, 0, 10);
            grid->addWidget(narratorListDisplay, 1, 0, 4, 3);
            grid->addWidget(progressBar, 1, 3, 1, 8);
            grid->addWidget(subScrollArea, 2, 3, 3, 8);
            grid->setRowMinimumHeight(0, 50);
            grid->setRowStretch(0, 0);
            grid->setRowStretch(2, 150);
            grid->setRowMinimumHeight(2, 500);
            grid->setRowMinimumHeight(3, 300);
            grid->setRowStretch(3, 0);
            //grid->setColumnStretch(0,150);
            //grid->setColumnStretch(7,150);
            subScrollArea->setWidget(pic);
            setCentralWidget(scrollArea);
            subScrollArea->setWidgetResizable(true);
            connect(find, SIGNAL(clicked()), this, SLOT(find_clicked()));
            connect(display, SIGNAL(clicked()), this, SLOT(display_clicked()));
            connect(biography, SIGNAL(clicked()), this, SLOT(biography_clicked()));
            setWindowTitle("Localized Graph Display");
            this->resize(1100, 700);
            hadith_text = new QString();
            hadith_out.setString(hadith_text);
            theSarf->out.setString(hadith_text);
            errors = new QTextBrowser(this);
            errors->resize(errors->width(), 50);
            errors_text = new QString();
            grid->addWidget(errors, 6, 0, 1, 11);
            theSarf->displayed_error.setString(errors_text);
            errors->setText(*errors_text);
        }

    public slots:
        void find_clicked() {
            find_action();
        }
        void display_clicked() {
            display_action();
        }
        void biography_clicked() {
            biography_action();
        }
    private:
        NarratorNodeIfc *getSelectedNode() {
            QList<QTableWidgetSelectionRange>  selection = narratorListDisplay->selectedRanges();
            NarratorNodeIfc *node = NULL;

            for (int i = 0; i < selection.size(); i++) {
                int topRow = selection[i].topRow();

                for (int j = 0; j < selection[i].rowCount(); j++) {
                    int row = topRow + j;
                    node = nodeList[row];
                }
            }

            return node;
        }

        void find_action();
        void display_action();
        void biography_action();
        static int searchNarratorsInHash(ChainsContainer &chains, ATMProgressIFC *prg, QString) {
            LocalizedDisplay *this_ = (LocalizedDisplay *)prg;
            int size = chains.size();

            for (int i = 0; i < size; i++) {
                Chain &c = *chains[i];
                int chainSize = c.m_chain.size();

                for (int j = 0; j < chainSize; j++) {
                    ChainPrim *chainPrim = c.m_chain[j];

                    if (chainPrim->isNarrator()) {
                        Narrator *n = dynamic_cast<Narrator *>(chainPrim);
                        ColorNarratorsAction c(this_->nodeMap);
                        this_->graph->performActionToAllCorrespondingNodes(n, c);
                    }
                }
            }

            return 0;
        }

        virtual void report(int value) {
            progressBar->setValue(value);
        }
        virtual void tag(int , int , QColor , bool) {}
        virtual void startTaggingText(QString &) {}
        virtual void finishTaggingText() {}
        virtual void setCurrentAction(const QString &s) {
            progressBar->setFormat(s + "(%p%)");
        }
        virtual void resetActionDisplay() {
            progressBar->setFormat("%p%");
            progressBar->reset();
        }
        virtual QString getFileName() {
            return "";
        }

        class ColorNarratorsAction: public NarratorHash::FoundAction {
            public:
                typedef QMap<NarratorNodeIfc *, double> DetectedNodesMap;
            private:
                DetectedNodesMap &map;
            public:
                ColorNarratorsAction(DetectedNodesMap &m): map(m) { }
                virtual void action(const QString & /*s*/, GroupNode *node, double v) {
                    NarratorNodeIfc *n = &node->getCorrespondingNarratorNode();
                    DetectedNodesMap::iterator i = map.find(n);

                    if (i != map.end()) {
                        double oldSimilarity = *i;

                        if (oldSimilarity < v) {
                            map[n] = v;
                        }
                    } else {
                        map[n] = v;
                    }
                }
        };

    private:

        ColorNarratorsAction::DetectedNodesMap nodeMap;
        QPushButton *find, *display, * biography;
        QLabel *steps_label, *input_label;
        QTextEdit *steps;
        QTextBrowser *text;
        QScrollArea *scrollArea, * subScrollArea;
        QTextEdit *input;
        QLabel *pic;
        QProgressBar *progressBar;
        QTableWidget *narratorListDisplay;
        QList<NarratorNodeIfc *> nodeList;
        QTextBrowser *errors;
        QString *errors_text, *hadith_text;
        QGridLayout *grid;

        NarratorGraph *graph;

        ~LocalizedDisplay() {
            delete find;
            delete display;
            delete biography;
            delete input;
            delete input_label;
            delete scrollArea;
            delete subScrollArea;
            delete pic;
            delete progressBar;
            delete narratorListDisplay;
            delete hadith_text;
            delete errors;
            delete errors_text;
            delete grid;
            delete graph;
        }
};

void localizedDisplay(NarratorGraph *graph);

#endif // LOCALIZEDDISPLAY_H
