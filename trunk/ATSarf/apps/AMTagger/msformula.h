#ifndef MSFORMULA_H
#define MSFORMULA_H

#include <QVector>
#include <QMap>
#include "mbf.h"
#include "unaryf.h"
#include "binaryf.h"
#include "sequetialf.h"
#include "relation.h"

class MSFormula: public MSF
{
public:
    /// Constructor taking name and parent of MSF
    MSFormula(QString name, MSF* parent);
    /// Add an MSF to a given parent
    bool addMSF(/*QString parent, */MSF* msf/*,int left=-1*/);
    /// Remove an MSF from formula
    bool removeMSF(QString parent, QString msfName);
    /// Unary update MSFs
    bool updateMSF(QString parent, QString child, UNARYF* msf);
    /// Binary update MSFs
    bool updateMSF(QString parent, QString fchild, QString schild, BINARYF* msf);
    /// Formula update MSFs
    bool updateMSF(QString parent, QVector<QString>* children, SequentialF* msf);
    /// Vector containing the sequence of MSFs
    QVector<MSF*> vector;
    /// Vector containing the formula relations
    QVector<Relation*> relationVector;
    /// Map that takes formula name and returns its structure
    QMap<QString,MSF*> map;
    /// Map to keep list of parameters to each function
    QMultiMap<QString,QPair<QString,QString> > functionParametersMap;
    /// Counter for msf names
    int i;
    /// Formula Description
    QString description;
    /// Foreground Color
    QString fgcolor;
    /// Background Color
    QString bgcolor;
    /// Headers included in actions
    QString includes;
    /// Members used in actions
    QString members;
    /// String to save all functions and function calls for actions
    QString actionData;
    bool isMBF();
    bool isUnary();
    bool isBinary();
    bool isFormula();
    bool isSequential();
    QString print();
    QString printwithNames();
    void buildTree(QTreeWidget* parent);
    void buildTree(QTreeWidgetItem* parent);
    bool buildActionFile(QString &actionsData, QMultiMap<QString, QPair<QString,QString> > *functionParametersMap);
    QVariantMap getJSON();
    bool buildNFA(NFA *nfa);
    int usedCount;
    bool removeSelfFromMap(QMap<QString, MSF*> &map);
    QStringList getMSFNames();
    ~MSFormula();
};

#endif // MSFORMULA_H
