#ifndef DECLARATIONSVIEW_H
#define DECLARATIONSVIEW_H

#include <QMainWindow>
#include <QTextEdit>
#include "includesview.h"

class DeclarationsView : public QMainWindow
{
    Q_OBJECT
public:
    explicit DeclarationsView(MSFormula *currentF, bool *isDirty, QWidget *parent = 0);

signals:

private slots:
    void declarations_edited();

private:
    QTextEdit *editDeclarations;

    MSFormula *currentF;
    bool *isDirty;
    QString msfName;
};

#endif // DECLARATIONSVIEW_H
