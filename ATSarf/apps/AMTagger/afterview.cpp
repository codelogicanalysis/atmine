#include "afterview.h"

AfterView::AfterView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent) :
    QMainWindow(parent)
{
    this->currentF = currentF;
    this->isDirty = isDirty;
    this->msfName = msfName;

    /** Build View **/
    editAfter = new QTextEdit(this);

    setCentralWidget(editAfter);
    setWindowTitle(tr("Post-Match Action View"));

    editAfter->setText(currentF->map.value(msfName)->after);

    connect(editAfter, SIGNAL(textChanged()), this, SLOT(after_edited()));
}

void AfterView::after_edited() {
    MSF* msf = currentF->map.value(msfName);
    msf->after = editAfter->toPlainText();
    *isDirty = true;
}
