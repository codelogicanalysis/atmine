#ifndef HADITH_H
#define HADITH_H

#include <QString>
#include "../common_structures/common.h"
#define GENERAL_HADITH
//#define HADITHDEBUG  //just for additional debugging statements displayed
//#define TENTATIVE //are decisions that we are not sure about


#ifdef GUI_SPECIFIC
#include "../gui/mainwindow.h"
int hadith(QString input_str,Ui::MainWindow *m_ui);
#else
int hadith(QString input_str);
#endif
void hadith_initialize();

#endif // HADITH_H
