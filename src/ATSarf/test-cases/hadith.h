#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "../gui/mainwindow.h"
#define GENERAL_HADITH
//#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about

int hadith(QString input_str,Ui::MainWindow *m_ui);
void hadith_initialize();

#endif // HADITH_H
