#ifndef ACTIONSVIEW_H
#define ACTIONSVIEW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "includesview.h"
#include "declarationsview.h"
//#include "afterview.h"
#include "initview.h"

class ActionsView : public QMainWindow
{
    Q_OBJECT
public:
    explicit ActionsView(MSFormula *currentF, QString msfName, bool *isDirty, QWidget *parent = 0);

signals:

private slots:
    void btnText_clicked();
    void btnPosition_clicked();
    void btnLength_clicked();
    void btnNumber_clicked();
    void btnIncludes_clicked();
    void btnDeclarations_clicked();
    void btnInit_clicked();
    //void btnAfter_clicked();
    void actions_edited();
    void return_edited(QString);

private:
    QLabel *lblVariables;
    QLabel *lblFormula;
    QLabel* lblReturn;

    QLineEdit *editFormula;
    QLineEdit *editReturn;

    QTextEdit *editActions;

    QListWidget *listVariables;

    QPushButton *btnText;
    QPushButton *btnPosition;
    QPushButton *btnLength;
    QPushButton *btnNumber;
    QPushButton *btnInit;
    //QPushButton *btnAfter;
    QPushButton *btnDeclarations;
    QPushButton *btnIncludes;

    MSFormula *currentF;
    bool *isDirty;
    QString msfName;

    void disconnect_Signals();
    void connect_Signals();
};

#endif // ACTIONSVIEW_H
