#ifndef RELATIONSVIEW_H
#define RELATIONSVIEW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include "msformula.h"

class RelationsView : public QMainWindow
{
    Q_OBJECT
public:
    explicit RelationsView(MSFormula* formula, bool *isDirty, QWidget *parent = 0);

private slots:
    void btnAdd_clicked();
    void btnRemove_clicked();
    void cbName_changed(QString);
    void cbEdge_changed(QString);
    void cbE1_changed(QString);
    void cbE2_changed(QString);
    void cbEdgeLabel_changed(QString);
    void cbE1Label_changed(QString);
    void cbE2Label_changed(QString);
    void closeEvent(QCloseEvent *event);

private:
    QLabel* lblName;
    QLabel* lblEdge;
    QLabel* lblE1;
    QLabel* lblE2;
    QLabel* lblMatch1;
    QLabel* lblMatch2;
    QLabel* lblNodeLabel;
    QLabel* lblEdgeLabel;

    QPushButton* btnAdd;
    QPushButton* btnRemove;

    QComboBox* cbName;
    QComboBox* cbEdge;
    QComboBox* cbE1;
    QComboBox* cbE2;
    QComboBox* cbEdgeLabel;
    QComboBox* cbE1Label;
    QComboBox* cbE2Label;

    QTreeWidget* treeMSF;

    QWidget* parent;
    bool* isDirty;
    MSFormula* formula;
    Relation* currentR;
    QVector<Relation*> vector;
    void disconnect_Signals();
    void connect_Signals();
};

#endif // RELATIONSVIEW_H
