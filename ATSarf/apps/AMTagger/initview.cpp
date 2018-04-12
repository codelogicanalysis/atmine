#include "initview.h"

InitView::InitView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent) :
    QMainWindow(parent)
{
    this->currentF = currentF;
    this->msfName = msfName;
    this->isDirty = isDirty;

    /** Build View **/
    editInit = new QTextEdit(this);

    setCentralWidget(editInit);
    setWindowTitle(tr("Pre-Match Action View"));

    editInit->setText(currentF->map.value(msfName)->init);

    connect(editInit, SIGNAL(textChanged()), this, SLOT(init_edited()));
}

void InitView::init_edited() {
    MSF* msf = currentF->map.value(msfName);
    msf->init = editInit->toPlainText();
    *isDirty = true;
}
