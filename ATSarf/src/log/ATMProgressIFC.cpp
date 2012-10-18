#include "ATMProgressIFC.h"
#include <QString>
#include <QtGui/QColor>

void EmptyProgressIFC::report(int ) {}
void EmptyProgressIFC::startTaggingText(QString & ) {}
void EmptyProgressIFC::tag(int , int ,QColor , bool ) {}
void EmptyProgressIFC::finishTaggingText() {}
void EmptyProgressIFC::setCurrentAction(const QString & ){}
void EmptyProgressIFC::resetActionDisplay(){}
QString EmptyProgressIFC::getFileName() {return "";}
