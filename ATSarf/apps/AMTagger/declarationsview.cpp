#include "declarationsview.h"

DeclarationsView::DeclarationsView(MSFormula *currentF, bool *isDirty, QWidget *parent) :
    QMainWindow(parent)
{
    this->currentF = currentF;
    this->isDirty = isDirty;
    this->msfName = msfName;

    /** Build View **/
   editDeclarations = new QTextEdit(this);

    setCentralWidget(editDeclarations);
    setWindowTitle(tr("Variable Declarations View"));

    editDeclarations->setText(currentF->members);

    connect(editDeclarations, SIGNAL(textChanged()), this, SLOT(declarations_edited()));
}

void DeclarationsView::declarations_edited() {
    currentF->members = editDeclarations->toPlainText();
    *isDirty = true;
}
