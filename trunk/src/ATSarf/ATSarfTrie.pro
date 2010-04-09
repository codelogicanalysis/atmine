TOP = ../..
INCLUDEPATH += $$QTDIR/include/QtSql \
    $$QTDIR/include/Qt
INCLUDEPATH += $$TOP/datrie/
INCLUDEPATH += $$TOP/datrie/libdatrie-0.2.2
INCLUDEPATH += $$TOP/datrie/libdatrie-0.2.2/datrie
HEADERS += utilities/text_handling.h \
    utilities/diacritics.h \
    utilities/dbitvec.h \
    sql-interface/sql_queries.h \
    sql-interface/Search_Compatibility.h \
    sql-interface/Search_by_item.h \
    sql-interface/Search_by_category.h \
    sarf/tree_search.h \
    sarf/suffix_search.h \
    sarf/stemmer.h \
    sarf/stem_search.h \
    sarf/prefix_search.h \
    common_structures/tree.h \
    common_structures/result_node.h \
    common_structures/node.h \
    common_structures/letter_node.h \
    caching_structures/database_info_block.h \
    caching_structures/compatibility_rules.h \
    builders/functions.h \
    gui/test.h \
    gui/mainwindow.h \
    logger/logger.h \
    common_structures/common.h \
    sql-interface/Retrieve_Template.h \
    StemNode.h
HEADERS += atmTrie.h \
    trie.h \
    utilities.h
SOURCES += utilities/text_handling.cpp \
    utilities/diacritics.cpp \
    utilities/dbitvec.cpp \
    sql-interface/sql_queries.cpp \
    sql-interface/Search_Compatibility.cpp \
    sql-interface/Search_by_item.cpp \
    sql-interface/Search_by_category.cpp \
    sarf/suffix_search.cpp \
    sarf/stemmer.cpp \
    sarf/stem_search.cpp \
    sarf/prefix_search.cpp \
    common_structures/tree.cpp \
    common_structures/result_node.cpp \
    common_structures/node.cpp \
    common_structures/letter_node.cpp \
    caching_structures/database_info_block.cpp \
    caching_structures/compatibility_rules.cpp \
    builders/functions.cpp \
    gui/test.cpp \
    gui/mainwindow.cpp \
    sarf/tree_search.cpp \
    sql-interface/Retrieve_Template.cpp \
    common_structures/global_variables.cpp \
    StemNode.cpp
SOURCES += 

# LIBS += -L$$TOP/datrie/libdatrie-0.2.2/datrie/.libs -ldatrie
# LIBS += -L$$TOP/datrie/libdatrie-0.2.2/datrie/.libs -L. -ldatrie
CONFIG += qt
CONFIG += debug_and_release

# debug
QT += sql
FORMS += gui/mainwindow.ui
