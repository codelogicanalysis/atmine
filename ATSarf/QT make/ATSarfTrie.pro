TOP = ../third/
INCLUDEPATH += $$QTDIR/include/QtSql \
    $$QTDIR/include/Qt \
    ../src/common \
    ../src/util \
    ../apps/gui \
    ../src/sarf \
    ../apps/case \
    ../src/log \
    ../src/cache \
    ../src/sql \
    ../src/build

INCLUDEPATH += $$TOP/datrie/
INCLUDEPATH += $$TOP/datrie/datrie

HEADERS += ../src/util/diacritics.h \
    ../src/util/dbitvec.h \
    ../src/sql/sql_queries.h \
    ../src/sql/Search_Compatibility.h \
    ../src/sql/Search_by_item.h \
    ../src/sql/Search_by_category.h \
    ../src/sarf/tree_search.h \
    ../src/sarf/suffix_search.h \
    ../src/sarf/stemmer.h \
    ../src/sarf/stem_search.h \
    ../src/sarf/prefix_search.h \
    ../src/common/tree.h \
    ../src/common/result_node.h \
    ../src/common/node.h \
    ../src/common/letter_node.h \
    ../src/cache/database_info_block.h \
    ../src/cache/compatibility_rules.h \
    ../src/build/functions.h \
    ../src/log/logger.h \
    ../src/common/common.h \
    ../src/sql/Retrieve_Template.h \
    ../src/common/StemNode.h \
    ../src/common/atmTrie.h \
    ../src/util/letters.h \
    ../src/log/ATMProgressIFC.h \
    ../src/util/Math_functions.h \
    ../src/cache/Search_by_item_locally.h \
    ../src/util/Triplet.h \
    ../src/util/Ptr.h \
    ../src/common/node_info.h \
    ../src/common/solution_position.h \
    ../src/util/reference.h \
    ../src/util/text_handling.h \
    ../src/util/Quadruplet.h \
    ../src/util/textParsing.h \
    ../src/util/mergeLists.h \
    ../src/util/editDistance.h \
    ../src/common/inflections.h \
    ../src/util/transliteration.h \
    ../src/util/vocalizedCombinations.h \
    ../src/util/ambiguity.h \
    ../src/util/morphemes.h \
    ../src/util/combinations.h \
    ../src/common/sarf.h \
    ../apps/case/twoLevelTaggerSelection.h \
    ../apps/case/timeRecognizer.h \
    ../apps/case/timeManualTagger.h \
    ../apps/case/split_affixes.h \
    ../apps/case/OneLevelAgreement.h \
    ../apps/case/narratorHash.h \
    ../apps/case/narratorEqualityModel.h \
    ../apps/case/narratordetector.h \
    ../apps/case/narrator_abstraction.h \
    ../apps/case/localizeddisplay.h \
    ../apps/case/hadith_utilities.h \
    ../apps/case/hadithManualTagger.h \
    ../apps/case/hadithInterAnnotatorAgreement.h \
    ../apps/case/hadithDagItemModel.h \
    ../apps/case/hadithDagGraph.h \
    ../apps/case/hadithCommon.h \
    ../apps/case/hadithChainItemModel.h \
    ../apps/case/hadithChainGraph.h \
    ../apps/case/hadith.h \
    ../apps/case/graph_nodes.h \
    ../apps/case/graph.h \
    ../apps/case/geneMainwindow.h \
    ../apps/case/genealogyItem.h \
    ../apps/case/genealogyInterAnnotatorAgreement.h \
    ../apps/case/diacriticsPostProcessing.h \
    ../apps/case/diacriticDisambiguation.h \
    ../apps/case/biographyGraphUtilities.h \
    ../apps/case/biographies.h \
    ../apps/case/bibleManualTagger.h \
    ../apps/case/bibleGeneology.h \
    ../apps/case/atbExperiment.h \
    ../apps/case/atbDiacriticExperiment.h \
    ../apps/case/affixTreeTraversal.h \
    ../apps/case/abstractTwoLevelAnnotator.h \
    ../apps/case/AbstractTwoLevelAgreement.h \
    ../apps/case/abstractGraph.h \
    ../apps/case/abstractAnnotator.h \
    ../apps/gui/test.h \
    ../apps/gui/mainwindow.h \
    ../src/common/TreeVisitor.h \
    ../src/common/TreeFilter.h \
    ../src/common/Tree2Dot.h \
    ../src/util/largeFileIterator.h \
    ../src/util/iterativeStandardDeviation.h \
    ../src/util/iterativeMathFunctions.h \
    ../src/util/instanceIterator.h \
    ../src/util/distinguishingLargeFileIterator.h \
    ../src/util/decisionTreeRegression.h
HEADERS += $$TOP/datrie/datrie/trie.h

SOURCES += ../src/util/text_handling.cpp \
    ../src/util/diacritics.cpp \
    ../src/util/dbitvec.cpp \
    ../src/sql/sql_queries.cpp \
    ../src/sql/Search_Compatibility.cpp \
    ../src/sql/Search_by_item.cpp \
    ../src/sql/Search_by_category.cpp \
    ../src/sarf/suffix_search.cpp \
    ../src/sarf/stemmer.cpp \
    ../src/sarf/stem_search.cpp \
    ../src/sarf/prefix_search.cpp \
    ../src/common/tree.cpp \
    ../src/common/result_node.cpp \
    ../src/common/node.cpp \
    ../src/common/letter_node.cpp \
    ../src/cache/database_info_block.cpp \
    ../src/cache/compatibility_rules.cpp \
    ../src/build/functions.cpp \
    ../src/sarf/tree_search.cpp \
    ../src/sql/Retrieve_Template.cpp \
    ../src/common/global_variables.cpp \
    ../src/common/StemNode.cpp \
    ../src/common/atmTrie.cpp \
    ../src/util/Math_functions.cpp \
    ../src/cache/Search_by_item_locally.cpp \
    ../src/common/common.cpp \
    ../src/common/solution_position.cpp \
    ../src/util/editDistance.cpp \
    ../src/util/transliteration.cpp \
    ../src/log/ATMProgressIFC.cpp \
    ../src/util/vocalizedCombinations.cpp \
    ../src/util/morphemes.cpp \
    ../src/util/ambiguity.cpp \
    ../src/util/combinations.cpp \
    ../src/common/sarf.cpp \
    ../apps/case/twoLevelTaggerSelection.cpp \
    ../apps/case/timeRecognizer.cpp \
    ../apps/case/timeManualTagger.cpp \
    ../apps/case/split_affixes.cpp \
    ../apps/case/regressionDiacriticsTest.cpp \
    ../apps/case/OneLevelAgreement.cpp \
    ../apps/case/narratorHash.cpp \
    ../apps/case/narratorEqualityModel.cpp \
    ../apps/case/narratorEqualityComparision.cpp \
    ../apps/case/narratordetector.cpp \
    ../apps/case/narrator_abstraction.cpp \
    ../apps/case/madaExperiment.cpp \
    ../apps/case/localizeddisplay.cpp \
    ../apps/case/hadithRefactored.cpp \
    ../apps/case/hadithManualTagger.cpp \
    ../apps/case/hadithInterAnnotatorAgreement.cpp \
    ../apps/case/hadithDagItemModel.cpp \
    ../apps/case/hadithDagGraph.cpp \
    ../apps/case/hadithCommon.cpp \
    ../apps/case/hadithChainItemModel.cpp \
    ../apps/case/hadithChainGraph.cpp \
    ../apps/case/graph_nodes.cpp \
    ../apps/case/graph.cpp \
    ../apps/case/geneMainwindow.cpp \
    ../apps/case/genealogyItem.cpp \
    ../apps/case/genealogyInterAnnotatorAgreement.cpp \
    ../apps/case/diacriticsPostProcessing.cpp \
    ../apps/case/diacriticDisambiguation.cpp \
    ../apps/case/biographyGraphUtilities.cpp \
    ../apps/case/biographies.cpp \
    ../apps/case/bibleManualTagger.cpp \
    ../apps/case/bibleGeneology.cpp \
    ../apps/case/atbExperiment.cpp \
    ../apps/case/atbDiacriticExperiment.cpp \
    ../apps/case/affix_verification.cpp \
    ../apps/case/abstractTwoLevelAnnotator.cpp \
    ../apps/case/AbstractTwoLevelAgreement.cpp \
    ../apps/case/abstractAnnotator.cpp \
    ../apps/gui/test.cpp \
    ../apps/gui/mainwindow.cpp \
    ../src/util/decisionTreeRegression.cpp \
    ../src/util/distinguishingLargeFileIterator.cpp \
    ../src/util/largeFileIterator.cpp \
    ../src/util/iterativeMathFunctions.cpp

# LIBS += -ldatrie
LIBS += -L$$TOP/datrie/datrie/.libs/ \
    -ldatrie
CONFIG += qt
CONFIG += debug_and_release

# -Q suppresses the call graph data
# QMAKE_CFLAGS += -pg
# QMAKE_CXXFLAGS += -pg
# QMAKE_LFLAGS += -pg
# QMAKE_CFLAGS += -O3
# QMAKE_CXXFLAGS += -O3
# QMAKE_LFLAGS += -O3
# debug
QT += sql
FORMS += ../apps/gui/mainwindow.ui
OTHER_FILES +=
