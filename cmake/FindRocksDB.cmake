# Find rocksdb
#
# Find the rocksdb includes and library
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  ROCKSDB_INCLUDE_DIRS, where to find header, etc.
#  ROCKSDB_LIBRARIES, the libraries needed to use rocksdb.
#  ROCKSDB_FOUND, If false, do not try to use rocksdb.

# only look in default directories
find_path(
	ROCKSDB_INCLUDE_DIR
	NAMES rocksdb/db.h
	DOC "rocksdb include dir"
)

find_library(
	ROCKSDB_LIBRARY
	NAMES rocksdb
	DOC "rocksdb library"
)

set(ROCKSDB_INCLUDE_DIRS ${ROCKSDB_INCLUDE_DIR})
set(ROCKSDB_LIBRARIES ${ROCKSDB_LIBRARY})

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	find_library(
		ROCKSDB_LIBRARY_DEBUG
		NAMES rocksdbd
		DOC "rocksdb debug library"
	)

	set(ROCKSDB_LIBRARIES optimized ${ROCKSDB_LIBRARIES} debug ${ROCKSDB_LIBRARY_DEBUG})

endif()

# handle the QUIETLY and REQUIRED arguments and set ROCKSDB_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB DEFAULT_MSG
	ROCKSDB_INCLUDE_DIR ROCKSDB_LIBRARY)
mark_as_advanced (ROCKSDB_INCLUDE_DIR ROCKSDB_LIBRARY)

