#include "usercrossrelationview.h"

UserCrossRelationView::UserCrossRelationView(bool* dirty)
{
    this->dirty = dirty;

    QGridLayout *grid = new QGridLayout();

    lblEntity1 = new QLabel(tr("Entity 1:"));
    lblEntity2 = new QLabel(tr("Entity 2:"));
    lblUserRelation = new QLabel(tr("User Relation"));

    btnTextE1 = new QPushButton(tr("Text"), this);
    btnPosE1 = new QPushButton(tr("POS"), this);
    btnLengthE1 = new QPushButton(tr("Length"), this);
    btnNumberE1 = new QPushButton(tr("Number"), this);

    btnTextE2 = new QPushButton(tr("Text"), this);
    btnPosE2 = new QPushButton(tr("POS"), this);
    btnLengthE2 = new QPushButton(tr("Length"), this);
    btnNumberE2 = new QPushButton(tr("Number"), this);

    editCrossRelation = new QTextEdit(this);

    grid->addWidget(lblEntity1,0,0);
    grid->addWidget(btnTextE1,1,0);
    grid->addWidget(btnPosE1,2,0);
    grid->addWidget(btnLengthE1,3,0);
    grid->addWidget(btnNumberE1,4,0);

    grid->addWidget(lblEntity2,0,2);
    grid->addWidget(btnTextE2,1,2);
    grid->addWidget(btnPosE2,2,2);
    grid->addWidget(btnLengthE2,3,2);
    grid->addWidget(btnNumberE2,4,2);

    grid->addWidget(lblUserRelation,0,1,1,1,Qt::AlignCenter);
    grid->addWidget(editCrossRelation,1,1,4,1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle("User-defined Cross Relation");

    if(!(_atagger->userCrossRelation.isEmpty())) {
        editCrossRelation->setText(_atagger->userCrossRelation);
    }

    connect_Signals();
}

void UserCrossRelationView::btnTextE1_clicked() {

    QString text = "$e1.text";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnPositionE1_clicked() {

    QString text = "$e1.position";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnLengthE1_clicked() {

    QString text = "$e1.length";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnNumberE1_clicked() {

    QString text = "$e1.number";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnTextE2_clicked() {

    QString text = "$e2.text";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnPositionE2_clicked() {

    QString text = "$e2.position";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnLengthE2_clicked() {

    QString text = "$e2.length";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::btnNumberE2_clicked() {

    QString text = "$e2.number";
    editCrossRelation->textCursor().insertText(text);
}

void UserCrossRelationView::disconnect_Signals() {
    disconnect(btnTextE1, SIGNAL(clicked()), this, SLOT(btnTextE1_clicked()));
    disconnect(btnPosE1, SIGNAL(clicked()), this, SLOT(btnPositionE1_clicked()));
    disconnect(btnLengthE1, SIGNAL(clicked()), this, SLOT(btnLengthE1_clicked()));
    disconnect(btnNumberE1, SIGNAL(clicked()), this, SLOT(btnNumberE1_clicked()));

    disconnect(btnTextE2, SIGNAL(clicked()), this, SLOT(btnTextE2_clicked()));
    disconnect(btnPosE2, SIGNAL(clicked()), this, SLOT(btnPositionE2_clicked()));
    disconnect(btnLengthE2, SIGNAL(clicked()), this, SLOT(btnLengthE2_clicked()));
    disconnect(btnNumberE2, SIGNAL(clicked()), this, SLOT(btnNumberE2_clicked()));
}

void UserCrossRelationView::connect_Signals() {
    connect(btnTextE1, SIGNAL(clicked()), this, SLOT(btnTextE1_clicked()));
    connect(btnPosE1, SIGNAL(clicked()), this, SLOT(btnPositionE1_clicked()));
    connect(btnLengthE1, SIGNAL(clicked()), this, SLOT(btnLengthE1_clicked()));
    connect(btnNumberE1, SIGNAL(clicked()), this, SLOT(btnNumberE1_clicked()));

    connect(btnTextE2, SIGNAL(clicked()), this, SLOT(btnTextE2_clicked()));
    connect(btnPosE2, SIGNAL(clicked()), this, SLOT(btnPositionE2_clicked()));
    connect(btnLengthE2, SIGNAL(clicked()), this, SLOT(btnLengthE2_clicked()));
    connect(btnNumberE2, SIGNAL(clicked()), this, SLOT(btnNumberE2_clicked()));
}

void UserCrossRelationView::closeEvent(QCloseEvent * /*event*/) {
    QString userDefRel = editCrossRelation->toPlainText();
    if(userDefRel != _atagger->userCrossRelation) {
        _atagger->userCrossRelation = editCrossRelation->toPlainText();
        *dirty = true;
    }
}
