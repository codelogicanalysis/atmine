#ifndef CUSTOMIZEMSFVIEW_H
#define CUSTOMIZEMSFVIEW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QGridLayout>
#include <QInputDialog>
#include <QStringList>
#include <QMessageBox>
#include <QtCore/QDir>
#include "global.h"
#include "logger.h"
#include "mbf.h"
#include "unaryf.h"
#include "binaryf.h"
#include "msformula.h"
#include "sequetialf.h"

class CustomizeMSFView : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomizeMSFView(QWidget *parent = 0);

signals:

private slots:
    void btnSelect_clicked();
    void btnUnselect_clicked();
    void btnAdd_clicked();
    void btnRemove_clicked();
    void btnStar_clicked();
    void btnPlus_clicked();
    void btnQuestion_clicked();
    void btnLimit_clicked();
    void btnOr_clicked();
    void btnAnd_clicked();
    void btnSequence_clicked();
    void cbMSF_changed(QString);
    void listMBF_itemclicked(QListWidgetItem*);

private:
    QPushButton *btnSelect;
    QPushButton *btnUnselect;
    QPushButton *btnAdd;
    QPushButton *btnRemove;
    QPushButton* btnStar;
    QPushButton* btnPlus;
    QPushButton* btnQuestion;
    QPushButton* btnLimit;
    QPushButton* btnOr;
    QPushButton* btnAnd;
    QPushButton* btnSequence;

    QLabel *lblMBF;
    QLabel *lblMSF;
    QLabel *lblActions;
    QLabel *lblFormula;

    QLineEdit *editLimit;
    QLineEdit *editFormula;
    QTextEdit *editActions;

    QComboBox *cbMSF;

    QListWidget *listMBF;

    QTreeWidget *treeMSF;
    QTreeWidget *treeMBFdesc;

    QStringList listCategoryId;
    QStringList listCategory;
    MSFormula* currentF;

    void disconnect_Signals();
    void connect_Signals();
};

#endif // CUSTOMIZEMSFVIEW_H
