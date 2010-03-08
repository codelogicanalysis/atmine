SOURCES += mainwindow.cpp
INCLUDEPATH += C:/Qt/2009.04/qt/include/QtSql \
	C:/Qt/2009.04/qt/include/Qt ../../dbitvec/
CONFIG += qt

# debug
QT += sql

# OTHER_FILES += Makefile
FORMS += mainwindow.ui
HEADERS += mainwindow.h \
    sql-interface.h \
    insert_scripts.h \
    utilities.h \
    tree.h \
    test.h \
    tree_search.h \
    ../../dbitvec/dbitvec.h \
    database_info.h
