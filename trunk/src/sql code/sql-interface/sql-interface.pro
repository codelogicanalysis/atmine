SOURCES += mainwindow.cpp
INCLUDEPATH += /usr/include/qt4/QtSql
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
