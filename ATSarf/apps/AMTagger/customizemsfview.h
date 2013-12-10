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
#include <QFileDialog>
#include <QScrollArea>
#include <QSpinBox>
#include "global.h"
#include "logger.h"
#include "mbf.h"
#include "unaryf.h"
#include "binaryf.h"
#include "msformula.h"
#include "sequetialf.h"
#include "colorlisteditor.h"
#include "actionsview.h"
#include "relationsview.h"

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
    void btnActions_clicked();
    void btnRelations_clicked();
    /*
    void init_edited();
    void after_edited();
    */
    void cbMSF_changed(QString);
    void listMBF_itemclicked(QListWidgetItem*);
    void closeEvent(QCloseEvent *event);
    void save();
    void fgcolor_changed(QString color);
    void bgcolor_changed(QString color);
    void description_edited();
    bool readMSF(MSFormula* formula, QVariant data, MSF *parent);

private:
    QPushButton *btnSelect;
    QPushButton *btnUnselect;
    QPushButton *btnAdd;
    QPushButton *btnRemove;
    QPushButton *btnStar;
    QPushButton *btnPlus;
    QPushButton *btnQuestion;
    QPushButton *btnLimit;
    QPushButton *btnOr;
    QPushButton *btnAnd;
    QPushButton *btnSequence;
    QPushButton *btnActions;
    QPushButton *btnRelations;

    QLabel *lblMBF;
    QLabel *lblMSF;
    QLabel *lblFormula;
    QLabel *lblDescription;
    QLabel *lblFGColor;
    QLabel *lblBGColor;

    QSpinBox *spinLimit;
    //QLineEdit *editLimit;
    QLineEdit *editFormula;

    QTextEdit *editDescription;

    QComboBox *cbMSF;

    QListWidget *listMBF;

    QTreeWidget *treeMSF;
    QTreeWidget *treeMBFdesc;

    ColorListEditor * colorfgcolor;
    ColorListEditor * colorbgcolor;

    QStringList listCategoryId;
    QStringList listCategory;
    QString currentFName;
    MSFormula* currentF;
    bool isDirty;

    void disconnect_Signals();
    void connect_Signals();
};

#endif // CUSTOMIZEMSFVIEW_H
