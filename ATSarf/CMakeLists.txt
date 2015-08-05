cmake_minimum_required(VERSION 2.8.11)
project(atmine)

# Fixing OS X rpath issues.
if(NOT DEFINED CMAKE_MACOSX_RPATH)
    set(CMAKE_MACOSX_RPATH 0)
endif(NOT DEFINED CMAKE_MACOSX_RPATH)

# Checking whether library directory on 64 bit linux systems has suffix 64.
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
if(LIB64 AND (CMAKE_SYSTEM_NAME STREQUAL Linux))
    set(LIB_SUFFIX 64)
else(LIB64 AND (CMAKE_SYSTEM_NAME STREQUAL Linux))
    set(LIB_SUFFIX "")
endif(LIB64 AND (CMAKE_SYSTEM_NAME STREQUAL Linux))

# Fixing install libdir on 64 bit linux machines.
set(INSTALL_LIB_DIR lib${LIB_SUFFIX} CACHE PATH "Installation directory for libraries")
mark_as_advanced(INSTALL_LIB_DIR)

#add_definitions("-fPIC")

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra")

# Finding required packages
find_package(PkgConfig REQUIRED)

# Finding Qt4 packages
find_package(Qt4 4.8.0 REQUIRED QtCore QtGui QtSql)

# Finding QJson
find_package(QJSON REQUIRED)

# Finding libdatrie
pkg_check_modules(DATRIE datrie-0.2 REQUIRED)

# Finding libcgraph
pkg_check_modules(CGRAPH libcgraph REQUIRED)

# Finding libgvc
pkg_check_modules(GVC libgvc REQUIRED)

# Include QT4 stuff
include(${QT_USE_FILE})

# Add subprojects
add_subdirectory(src)
add_subdirectory(apps)
