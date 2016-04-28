# Find miniupnpc
#
# Find the miniupnpc includes and library
#
# if you nee to add a custom library search path, do it via CMAKE_PREFIX_PATH
#
# This module defines
#  MINIUPNPC_INCLUDE_DIRS, where to find header, etc.
#  MINIUPNPC_LIBRARIES, the libraries needed to use miniupnpc.
#  MINIUPNPC_FOUND, If false, do not try to use miniupnpc.

# only look in default directories
find_path(
	MINIUPNPC_INCLUDE_DIR
	NAMES miniupnpc/miniupnpc.h
	DOC "miniupnpc include dir"
)

find_library(
	MINIUPNPC_LIBRARY
	NAMES miniupnpc
	DOC "miniupnpc library"
)

set(MINIUPNPC_INCLUDE_DIRS ${MINIUPNPC_INCLUDE_DIR})
set(MINIUPNPC_LIBRARIES ${MINIUPNPC_LIBRARY})

# debug library on windows
# same naming convention as in QT (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	find_library(
		MINIUPNPC_LIBRARY_DEBUG
		NAMES miniupnpcd
		DOC "miniupnpc debug library"
	)

	set(MINIUPNPC_LIBRARIES "iphlpapi" optimized ${MINIUPNPC_LIBRARIES} debug ${MINIUPNPC_LIBRARY_DEBUG})

endif()

# handle the QUIETLY and REQUIRED arguments and set MINIUPNPC_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(miniupnpc DEFAULT_MSG
	MINIUPNPC_INCLUDE_DIR MINIUPNPC_LIBRARY)
mark_as_advanced (MINIUPNPC_INCLUDE_DIR MINIUPNPC_LIBRARY)

