# Find leveldb
#
# Find the leveldb includes and library
# 
# if you need to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  LEVELDB_INCLUDE_DIRS, where to find header, etc.
#  LEVELDB_LIBRARIES, the libraries needed to use leveldb.
#  LEVELDB_FOUND, If false, do not try to use leveldb.

# only look in default directories
find_path(
	LEVELDB_INCLUDE_DIR 
	NAMES leveldb/db.h
	DOC "leveldb include dir"
)

find_library(
	LEVELDB_LIBRARY
	NAMES leveldb
	DOC "leveldb library"
)

set(LEVELDB_INCLUDE_DIRS ${LEVELDB_INCLUDE_DIR})
set(LEVELDB_LIBRARIES ${LEVELDB_LIBRARY})

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	find_library(
		LEVELDB_LIBRARY_DEBUG
		NAMES leveldbd
		DOC "leveldb debug library"
	)
	
	list(APPEND LEVELDB_LIBRARIES "shlwapi")
	list(APPEND LEVELDB_LIBRARY_DEBUG "shlwapi")
	set(LEVELDB_LIBRARIES optimized ${LEVELDB_LIBRARIES} debug ${LEVELDB_LIBRARY_DEBUG})
endif()

# handle the QUIETLY and REQUIRED arguments and set LEVELDB_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(leveldb DEFAULT_MSG
	LEVELDB_INCLUDE_DIR LEVELDB_LIBRARY)
mark_as_advanced (LEVELDB_INCLUDE_DIR LEVELDB_LIBRARY)

