INCLUDEPATH += /opt/qtsdk-2009.04/qt/include/QtSql \
    /opt/qtsdk-2009.04/qt/include/Qt
HEADERS += builders/functions.h \
    common_structures/common.h \
    logger/logger.h \
    sql-interface/sql_queries.h \
    sql-interface/Search_Compatibility.h \
    sql-interface/Search_by_item.h \
    sql-interface/Search_by_category.h \
    sql-interface/Retrieve_Template.h \
    utilities/text_handling.h \
    utilities/diacritics.h
SOURCES += builders/functions.cpp \
    builders/builder_main.cpp \
    common_structures/global_variables.cpp \
    sql-interface/sql_queries.cpp \
    sql-interface/Search_Compatibility.cpp \
    sql-interface/Search_by_item.cpp \
    sql-interface/Search_by_category.cpp \
    sql-interface/Retrieve_Template.cpp \
    utilities/text_handling.cpp \
    utilities/diacritics.cpp
CONFIG += qt

# debug
QT += sql
