#ifndef NFA_H
#define NFA_H

//#include "nfastate.h"
//#include <QString>
#include <QMultiMap>
#include <QStringList>
#include <QStack>
#include <QPair>
#include "msf.h"

class MSF;

class NFA
{
public:
    NFA(MSF* formula);
    MSF* formula;
    QString start;
    QStringList states;
    QString accept;
    QStringList andAccept;
    QStringList prioritySet;
    /// The last state added to the list of states
    QString last;
    QMultiMap<QString, QString> transitions;
    QMap<QString, QPair<MSF*,QString> > stateTOmsfMap;
    /// Stack to keep actions
    QStack<QString> *actionStack;
    int i;
};

#endif // NFA_H
