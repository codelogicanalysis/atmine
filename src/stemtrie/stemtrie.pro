TOP=/home/jad/Desktop/tools
QTDIR=/opt/qtsdk-2009.04/qt

HEADERS += atmTrieTest.h atmTrie.h trie.h
HEADERS += utilities.h

SOURCES += atmTrieTest.cpp \
    atmTrie.cpp main.cpp

#! [0]
RESOURCES     = stemtrie.qrc
#! [0]

INCLUDEPATH += $$QTDIR/include/Qt
INCLUDEPATH += $$QTDIR/include/QtCore
INCLUDEPATH += $$QTDIR/include/QtGui
INCLUDEPATH += $$QTDIR/include/QtSql
#INCLUDEPATH += $$TOP/datrie/
#INCLUDEPATH += $$TOP/datrie/libdatrie-0.2.2
#INCLUDEPATH += $$TOP/datrie/libdatrie-0.2.2/datrie
INCLUDEPATH += $$TOP/libdatrie-0.2.2
INCLUDEPATH += $$TOP/libdatrie-0.2.2/datrie

LIBS += -L$$TOP/libdatrie-0.2.2/datrie/.libs -ldatrie
CONFIG += qt
CONFIG += debug_and_release

# debug
QT += sql

# OTHER_FILES += Makefile
