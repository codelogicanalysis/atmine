#ifndef AFTERVIEW_H
#define AFTERVIEW_H

#include <QMainWindow>
#include <QTextEdit>
#include "msformula.h"

class AfterView : public QMainWindow
{
    Q_OBJECT
public:
    explicit AfterView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent = 0);

signals:

private slots:
    void after_edited();

private:
    QTextEdit *editAfter;

    MSFormula *currentF;
    bool *isDirty;
    QString msfName;
};

#endif // AFTERVIEW_H
