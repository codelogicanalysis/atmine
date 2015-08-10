cmake_minimum_required(VERSION 2.8.12)
project(case)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

get_filename_component(PARENT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PATH)
set(CASE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(CASE_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include_directories(${PARENT_DIRECTORY}/cache)
#include_directories(${PARENT_DIRECTORY}/case)
include_directories(${PARENT_DIRECTORY}/common)
include_directories(${PARENT_DIRECTORY}/log)
include_directories(${PARENT_DIRECTORY}/sarf)
include_directories(${PARENT_DIRECTORY}/sql)
include_directories(${PARENT_DIRECTORY}/util)

# UGLY, VICIOUS AND DANGEROUS
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../apps/gui)

# UGLY, HACK BECAUSE THE CODE IS POSITION ON FILESYSTEM DEPENDENT!!!!!!!
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/stop_words ${CMAKE_CURRENT_BINARY_DIR}/stop_words)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/phrases ${CMAKE_CURRENT_BINARY_DIR}/phrases)

# Run automoc and read source files.
set(CMAKE_AUTOMOC ON)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

# Add resource files
qt5_add_resources(RESOURCES resources.qrc)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${RESOURCES})
qt5_use_modules(${PROJECT_NAME} Core Gui Widgets Sql)
