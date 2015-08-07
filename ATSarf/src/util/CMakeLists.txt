cmake_minimum_required(VERSION 2.8.12)
project(util)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

get_filename_component(PARENT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PATH)
set(UTIL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(UTIL_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include_directories(${PARENT_DIRECTORY}/cache)
include_directories(${PARENT_DIRECTORY}/case)
include_directories(${PARENT_DIRECTORY}/common)
include_directories(${PARENT_DIRECTORY}/log)
include_directories(${PARENT_DIRECTORY}/sarf)
include_directories(${PARENT_DIRECTORY}/sql)
#include_directories(${PARENT_DIRECTORY}/util)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
qt5_use_modules(${PROJECT_NAME} Core Sql)
