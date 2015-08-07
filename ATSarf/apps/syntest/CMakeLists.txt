cmake_minimum_required(VERSION 2.8.12)
project(syntest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CACHE_INCLUDE_DIRS})
include_directories(${CASE_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDE_DIRS})
include_directories(${LOG_INCLUDE_DIRS})
include_directories(${SARF_INCLUDE_DIRS})
include_directories(${SQL_INCLUDE_DIRS})
include_directories(${UTIL_INCLUDE_DIRS})

include_directories(${SYN_INCLUDE_DIRS})

include_directories(${DATRIE_INCLUDE_DIRS})

link_directories(${CACHE_LIBRARY_DIRS})
link_directories(${CASE_LIBRARY_DIRS})
link_directories(${COMMON_LIBRARY_DIRS})
link_directories(${LOG_LIBRARY_DIRS})
link_directories(${SARF_LIBRARY_DIRS})
link_directories(${SQL_LIBRARY_DIRS})
link_directories(${UTIL_LIBRARY_DIRS})

link_directories(${SYN_LIBRARY_DIRS})

link_directories(${DATRIE_LIBRARY_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${DATRIE_LIBRARIES})
target_link_libraries(${PROJECT_NAME} cache)
target_link_libraries(${PROJECT_NAME} case)
target_link_libraries(${PROJECT_NAME} common)
target_link_libraries(${PROJECT_NAME} log)
target_link_libraries(${PROJECT_NAME} sarf)
target_link_libraries(${PROJECT_NAME} sql)
target_link_libraries(${PROJECT_NAME} util)
target_link_libraries(${PROJECT_NAME} syn)
