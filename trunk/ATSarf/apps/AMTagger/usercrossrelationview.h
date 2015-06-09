#ifndef USERCROSSRELATIONVIEW_H
#define USERCROSSRELATIONVIEW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include "global.h"

class UserCrossRelationView : public QMainWindow
{
    Q_OBJECT
public:
    UserCrossRelationView(bool* dirty);

private:
    bool* dirty;
    QLabel* lblEntity1;
    QLabel* lblEntity2;
    QLabel* lblUserRelation;

    QPushButton* btnTextE1;
    QPushButton* btnPosE1;
    QPushButton* btnLengthE1;
    QPushButton* btnNumberE1;

    QPushButton* btnTextE2;
    QPushButton* btnPosE2;
    QPushButton* btnLengthE2;
    QPushButton* btnNumberE2;

    QTextEdit* editCrossRelation;

    void disconnect_Signals();
    void connect_Signals();

private slots:
    void btnTextE1_clicked();
    void btnPositionE1_clicked();
    void btnLengthE1_clicked();
    void btnNumberE1_clicked();

    void btnTextE2_clicked();
    void btnPositionE2_clicked();
    void btnLengthE2_clicked();
    void btnNumberE2_clicked();

    void closeEvent(QCloseEvent *event);
};

#endif // USERCROSSRELATIONVIEW_H
