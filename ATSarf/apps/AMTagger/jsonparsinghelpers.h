#ifndef JSONPARSINGHELPERS_H
#define JSONPARSINGHELPERS_H

#include <QString>
#include <QVector>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include <QByteArray>
#include "word.h"
#include "msformula.h"
#include "sarftagtype.h"
#include "global.h"
#include "ger.h"
#include "autotagger.h"

void processText(QString *text);
bool process_TagTypes(QByteArray &tagtypedata);
bool readMSF(MSFormula* formula, QVariant data, MSF *parent);
bool readMatch(MSFormula* formula, QVariant data, Match* parent);
bool simulateMBF();

#endif // JSONPARSINGHELPERS_H
