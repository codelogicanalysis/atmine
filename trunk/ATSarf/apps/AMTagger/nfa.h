#ifndef NFA_H
#define NFA_H

//#include "nfastate.h"
//#include <QString>
#include <QMultiMap>
#include <QStringList>
#include <QStack>

class NFA
{
public:
    NFA(QString name);
    QString name;
    QString start;
    QStringList states;
    QString accept;
    QString andAccept;
    /// The last state added to the list of states
    QString last;
    QMultiMap<QString, QString> transitions;
    QMultiMap<QString, QString> stateTOmsfMap;
    /// Stack to keep actions
    QStack<QString> *actionStack;
    int i;
};

#endif // NFA_H
