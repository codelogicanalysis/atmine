SOURCES += mainwindow.cpp
INCLUDEPATH += /usr/include/qt4/QtSql ../
CONFIG += qt

# debug
QT += sql
# OTHER_FILES += Makefile
FORMS += mainwindow.ui
HEADERS +=  mainwindow.h \
	../sql-interface.h
