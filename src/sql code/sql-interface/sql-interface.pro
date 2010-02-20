SOURCES += mainwindow.cpp
INCLUDEPATH += C:/Qt/2009.04/qt/include/QtSql C:/Qt/2009.04/qt/include/Qt
CONFIG += qt

# debug
QT += sql

# OTHER_FILES += Makefile
FORMS += mainwindow.ui
HEADERS += mainwindow.h \
    sql-interface.h \
    insert_scripts.h \
    trie.h \
    utilities.h \
    tree.h \
    test.h
