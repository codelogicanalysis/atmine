#include "includesview.h"

IncludesView::IncludesView(MSFormula *currentF, bool *isDirty, QWidget *parent) :
    QMainWindow(parent)
{
    this->currentF = currentF;
    this->isDirty = isDirty;
    this->msfName = msfName;

    /** Build View **/
    editIncludes = new QTextEdit(this);

    setCentralWidget(editIncludes);
    setWindowTitle(tr("Include Libraries View"));

    editIncludes->setText(currentF->includes);

    connect(editIncludes, SIGNAL(textChanged()), this, SLOT(includes_edited()));
}

void IncludesView::includes_edited() {
    currentF->includes = editIncludes->toPlainText();
    *isDirty = true;
}
