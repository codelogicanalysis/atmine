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
    void cbMSF_changed(QString);
    void cbOperation_changed(QString);
    void listMBF_itemclicked(QListWidgetItem*);

private:
    QPushButton *btnSelect;
    QPushButton *btnUnselect;
    QPushButton *btnAdd;
    QPushButton *btnRemove;

    QLabel *lblMBF;
    QLabel *lblMSF;
    QLabel *lblActions;

    QLineEdit *editLimit;
    QTextEdit *editActions;

    QComboBox *cbMSF;
    QComboBox *cbOperations;

    QListWidget *listMBF;

    QTreeWidget *treeMSF;
    QTreeWidget *treeMBFdesc;

    QStringList listCategoryId;
    QStringList listCategory;

    void disconnect_Signals();
    void connect_Signals();
};

#endif // CUSTOMIZEMSFVIEW_H
