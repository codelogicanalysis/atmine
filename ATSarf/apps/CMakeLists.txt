cmake_minimum_required(VERSION 2.8.12)
project(atmine-apps)

# Add subprojects
add_subdirectory(ex0)
add_subdirectory(ex1)
add_subdirectory(ex2)
add_subdirectory(gui)
add_subdirectory(mtest)
add_subdirectory(syn)
add_subdirectory(numnorm)
add_subdirectory(sarfenumerate)
add_subdirectory(syntest)
add_subdirectory(worddensity)
add_subdirectory(demo)
add_subdirectory(AMTagger)

# Exporting include dirs
set(NUMNORM_INCLUDE_DIRS ${NUMNORM_INCLUDE_DIRS} PARENT_SCOPE)
set(SARFENUMERATE_INCLUDE_DIRS ${SYN_INCLUDE_DIRS} PARENT_SCOPE)
set(SYN_INCLUDE_DIRS ${SYN_INCLUDE_DIRS} PARENT_SCOPE)

# Exporting library dirs
set(NUMNORM_LIBRARY_DIRS ${NUMNORM_LIBRARY_DIRS} PARENT_SCOPE)
set(SARFENUMERATE_LIBRARY_DIRS ${SYN_LIBRARY_DIRS} PARENT_SCOPE)
set(SYN_LIBRARY_DIRS ${SYN_LIBRARY_DIRS} PARENT_SCOPE)

