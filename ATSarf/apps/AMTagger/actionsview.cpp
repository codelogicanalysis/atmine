#include "actionsview.h"

ActionsView::ActionsView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent) :
    QMainWindow(parent)
{
    this->currentF = currentF;
    this->isDirty = isDirty;
    this->msfName = msfName;

    QGridLayout *grid = new QGridLayout();

    lblVariables = new QLabel(tr("Variable List:"), this);
    lblFormula = new QLabel(tr("Formula:"), this);
    //lblReturn = new QLabel(tr("Returns:"), this);

    grid->addWidget(lblVariables, 0,3);
    grid->addWidget(lblFormula,0,0);
    //grid->addWidget(lblReturn,8,0);

    editFormula = new QLineEdit(this);
    editFormula->setReadOnly(true);
    //editReturn = new QLineEdit(this);

    grid->addWidget(editFormula,0,1,1,3);
    //grid->addWidget(editReturn,8,1,1,2);

    editActions = new QTextEdit(this);

    grid->addWidget(editActions,1,0,7,3);

    listVariables = new QListWidget(this);
    listVariables->setMaximumWidth(200);
    listVariables->setSelectionMode(QAbstractItemView::SingleSelection);

    grid->addWidget(listVariables,1,3,3,1);

    btnText = new QPushButton(tr("Text"), this);
    btnPosition = new QPushButton(tr("POS"), this);
    btnLength = new QPushButton(tr("Length"), this);
    btnNumber = new QPushButton(tr("Number"), this);
    btnInit = new QPushButton(tr("Pre-Match"), this);
    //btnAfter = new QPushButton(tr("Post-Match"), this);
    btnDeclarations = new QPushButton(tr("Declarations"), this);
    btnIncludes = new QPushButton(tr("Includes"), this);

    MSF* msf = currentF->map.value(msfName);
    if(!(msf->isMBF())) {
        btnText->setEnabled(false);
        btnLength->setEnabled(false);
        btnPosition->setEnabled(false);
        btnNumber->setEnabled(false);
    }

    grid->addWidget(btnText,4,3);
    grid->addWidget(btnPosition,5,3);
    grid->addWidget(btnLength,6,3);
    grid->addWidget(btnNumber,7,3);
    grid->addWidget(btnInit,9,2);
    //grid->addWidget(btnAfter,9,3);
    grid->addWidget(btnDeclarations,9,1);
    grid->addWidget(btnIncludes,9,0);

    QWidget *widget = new QWidget(this);
    widget->setLayout(grid);
    setCentralWidget(widget);
    setWindowTitle(tr("Formula Actions"));

    /** Fill Required Data **/

    editFormula->setText(currentF->map.value(msfName)->printwithNames());
    listVariables->addItems(currentF->map.value(msfName)->getMSFNames());
    editActions->setText(currentF->map.value(msfName)->actions);
    //editReturn->setText(currentF->map.value(msfName)->returns);

    connect_Signals();
}

void ActionsView::btnText_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(listVariables->currentItem()->text());
    text.append(".text");
    editActions->textCursor().insertText(text);
    *isDirty = true;
}

void ActionsView::btnPosition_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(listVariables->currentItem()->text());
    text.append(".position");
    editActions->textCursor().insertText(text);
    *isDirty = true;
}

void ActionsView::btnLength_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(listVariables->currentItem()->text());
    text.append(".length");
    editActions->textCursor().insertText(text);
    *isDirty = true;
}

void ActionsView::btnNumber_clicked() {
    if(currentF == NULL) {
        return;
    }

    QString text = "$";
    text.append(listVariables->currentItem()->text());
    text.append(".number");
    editActions->textCursor().insertText(text);
    *isDirty = true;
}

void ActionsView::btnIncludes_clicked() {
    IncludesView *iv = new IncludesView(currentF, isDirty, this);
    iv->show();
}

void ActionsView::btnDeclarations_clicked() {
    DeclarationsView *dv = new DeclarationsView(currentF, isDirty, this);
    dv->show();
}

void ActionsView::btnInit_clicked() {
    InitView *iv = new InitView(currentF, msfName, isDirty, this);
    iv->show();
}

/*
void ActionsView::btnAfter_clicked() {
    AfterView *av = new AfterView(currentF, msfName, isDirty, this);
    av->show();
}
*/

void ActionsView::actions_edited() {
    if(currentF == NULL) {
        return;
    }

    MSF* msf = currentF->map.value(msfName);
    msf->actions = editActions->toPlainText();
    *isDirty = true;
}

void ActionsView::return_edited(QString value) {
    if(currentF == NULL) {
        return;
    }

    MSF* msf = currentF->map.value(msfName);
    msf->returns = value;
    *isDirty = true;
}

void ActionsView::disconnect_Signals() {
    disconnect(btnText, SIGNAL(clicked()), this, SLOT(btnText_clicked()));
    disconnect(btnPosition, SIGNAL(clicked()), this, SLOT(btnPosition_clicked()));
    disconnect(btnLength, SIGNAL(clicked()), this, SLOT(btnLength_clicked()));
    disconnect(btnNumber, SIGNAL(clicked()), this, SLOT(btnNumber_clicked()));
    disconnect(editActions, SIGNAL(textChanged()), this, SLOT(actions_edited()));
    disconnect(btnIncludes, SIGNAL(clicked()), this, SLOT(btnIncludes_clicked()));
    disconnect(btnDeclarations, SIGNAL(clicked()), this, SLOT(btnDeclarations_clicked()));
    //disconnect(btnAfter, SIGNAL(clicked()), this, SLOT(btnAfter_clicked()));
    disconnect(btnInit, SIGNAL(clicked()), this, SLOT(btnInit_clicked()));
    //disconnect(editReturn, SIGNAL(textChanged(QString)), this, SLOT(return_edited(QString)));
}

void ActionsView::connect_Signals() {
    connect(btnText, SIGNAL(clicked()), this, SLOT(btnText_clicked()));
    connect(btnPosition, SIGNAL(clicked()), this, SLOT(btnPosition_clicked()));
    connect(btnLength, SIGNAL(clicked()), this, SLOT(btnLength_clicked()));
    connect(btnNumber, SIGNAL(clicked()), this, SLOT(btnNumber_clicked()));
    connect(editActions, SIGNAL(textChanged()), this, SLOT(actions_edited()));
    connect(btnIncludes, SIGNAL(clicked()), this, SLOT(btnIncludes_clicked()));
    connect(btnDeclarations, SIGNAL(clicked()), this, SLOT(btnDeclarations_clicked()));
    //connect(btnAfter, SIGNAL(clicked()), this, SLOT(btnAfter_clicked()));
    connect(btnInit, SIGNAL(clicked()), this, SLOT(btnInit_clicked()));
    //connect(editReturn, SIGNAL(textChanged(QString)), this, SLOT(return_edited(QString)));
}
