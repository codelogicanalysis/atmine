#ifndef INITVIEW_H
#define INITVIEW_H

#include <QMainWindow>
#include <QTextEdit>
#include "msformula.h"

class InitView : public QMainWindow
{
    Q_OBJECT
public:
    explicit InitView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent = 0);

signals:

private slots:
    void init_edited();

private:
    QTextEdit *editInit;

    MSFormula *currentF;
    QString msfName;
    bool *isDirty;
};

#endif // INITVIEW_H
