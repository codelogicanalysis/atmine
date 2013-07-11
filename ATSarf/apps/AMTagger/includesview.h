#ifndef INCLUDESVIEW_H
#define INCLUDESVIEW_H

#include <QMainWindow>
#include <QTextEdit>
#include "msformula.h"

class IncludesView : public QMainWindow
{
    Q_OBJECT
public:
    explicit IncludesView(MSFormula *currentF, bool *isDirty, QWidget *parent = 0);

signals:

private slots:
    void includes_edited();

private:
    QTextEdit *editIncludes;

    MSFormula *currentF;
    bool *isDirty;
    QString msfName;
};

#endif // INCLUDESVIEW_H
