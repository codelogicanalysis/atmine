#-------------------------------------------------
#
# Project created by QtCreator 2013-04-26T22:11:26
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = ANN
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$QTDIR/include/QtSql \
    $$QTDIR/include/Qt \
    .. \
    ../../../src/build \
    ../../../src/cache \
    ../../../src/common \
    ../../../src/log \
    ../../../src/sarf \
    ../../../src/sql \
    ../../../src/util \
    ../../../src/case \
    ../../syn \
    ../../../apps/gui

INCLUDEPATH += $$TOP/datrie/
INCLUDEPATH += $$TOP/datrie/datrie

SOURCES += \
    ../numnormmain.cpp \
    ../../../src/build/functions.cpp \
    ../../../src/cache/Search_by_item_locally.cpp \
    ../../../src/cache/database_info_block.cpp \
    ../../../src/cache/compatibility_rules.cpp \
    ../../../src/common/letter_node.cpp \
    ../../../src/common/common.cpp \
    ../../../src/common/atmTrie.cpp \
    ../../../src/common/tree.cpp \
    ../../../src/common/StemNode.cpp \
    ../../../src/common/solution_position.cpp \
    ../../../src/common/sarf.cpp \
    ../../../src/common/result_node.cpp \
    ../../../src/common/node.cpp \
    ../../../src/log/ATMProgressIFC.cpp \
    ../../../src/sarf/tree_search.cpp \
    ../../../src/sarf/suffix_search.cpp \
    ../../../src/sarf/stem_search.cpp \
    ../../../src/sarf/stemmer.cpp \
    ../../../src/sarf/prefix_search.cpp \
    ../../../src/sql/sql_queries.cpp \
    ../../../src/sql/Search_Compatibility.cpp \
    ../../../src/sql/Search_by_item.cpp \
    ../../../src/sql/Search_by_category.cpp \
    ../../../src/sql/Retrieve_Template.cpp \
    ../../../src/util/distinguishingLargeFileIterator.cpp \
    ../../../src/util/editDistance.cpp \
    ../../../src/util/iterativeMathFunctions.cpp \
    ../../../src/util/diacritics.cpp \
    ../../../src/util/ambiguity.cpp \
    ../../../src/util/combinations.cpp \
    ../../../src/util/dbitvec.cpp \
    ../../../src/util/dbitvec-test.cpp \
    ../../../src/util/decisionTreeRegression.cpp \
    ../../../src/util/largeFileIterator.cpp \
    ../../../src/util/text_handling.cpp \
    ../../../src/util/transliteration.cpp \
    ../../../src/util/vocalizedCombinations.cpp \
    ../../../src/util/Math_functions.cpp \
    ../../../src/util/morphemes.cpp \
    ../../../src/common/global_variables.cpp \
    ../../syn/getGloss.cpp \
    ../numnorm.cpp \
    ../number.cpp

HEADERS += \
    ../numnorm.h \
    ../myprogressifc.h \
    ../../../src/build/functions.h \
    ../../../src/cache/Search_by_item_locally.h \
    ../../../src/cache/database_info_block.h \
    ../../../src/cache/compatibility_rules.h \
    ../../../src/common/letter_node.h \
    ../../../src/common/inflections.h \
    ../../../src/common/common.h \
    ../../../src/common/atmTrie.h \
    ../../../src/common/TreeVisitor.h \
    ../../../src/common/TreeFilter.h \
    ../../../src/common/Tree2Dot.h \
    ../../../src/common/Tree.h \
    ../../../src/common/tree.h \
    ../../../src/common/StemNode.h \
    ../../../src/common/solution_position.h \
    ../../../src/common/sarf.h \
    ../../../src/common/result_node.h \
    ../../../src/common/node_info.h \
    ../../../src/common/node.h \
    ../../../src/log/logger.h \
    ../../../src/log/ATMProgressIFC.h \
    ../../../src/sarf/tree_search.h \
    ../../../src/sarf/suffix_search.h \
    ../../../src/sarf/stem_search.h \
    ../../../src/sarf/stemmer.h \
    ../../../src/sarf/prefix_search.h \
    ../../../src/sarf/MMB.h \
    ../../../src/sql/sql_queries.h \
    ../../../src/sql/Search_Compatibility.h \
    ../../../src/sql/Search_by_item.h \
    ../../../src/sql/Search_by_category.h \
    ../../../src/sql/Retrieve_Template.h \
    ../../../src/util/vocalizedCombinations.h \
    ../../../src/util/diacritics.h \
    ../../../src/util/distinguishingLargeFileIterator.h \
    ../../../src/util/editDistance.h \
    ../../../src/util/instanceIterator.h \
    ../../../src/util/decisionTreeRegression.h \
    ../../../src/util/ambiguity.h \
    ../../../src/util/combinations.h \
    ../../../src/util/dbitvec.h \
    ../../../src/util/iterativeMathFunctions.h \
    ../../../src/util/iterativeStandardDeviation.h \
    ../../../src/util/text_handling.h \
    ../../../src/util/textParsing.h \
    ../../../src/util/transliteration.h \
    ../../../src/util/Triplet.h \
    ../../../src/util/reference.h \
    ../../../src/util/Quadruplet.h \
    ../../../src/util/Ptr.h \
    ../../../src/util/largeFileIterator.h \
    ../../../src/util/letters.h \
    ../../../src/util/Math_functions.h \
    ../../../src/util/mergeLists.h \
    ../../../src/util/morphemes.h \
    ../../syn/getGloss.h \
    ../number.h \
    ../word.h

LIBS += -L$$TOP/datrie/datrie/.libs/ \
    -ldatrie
