# Find gmp
#
# Find the gmp includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  GMP_INCLUDE_DIRS, where to find header, etc.
#  GMP_LIBRARIES, the libraries needed to use gmp.
#  GMP_FOUND, If false, do not try to use gmp.

# only look in default directories
find_path(
	GMP_INCLUDE_DIR 
	NAMES gmp.h
	DOC "gmp include dir"
)

find_library(
	GMP_LIBRARY
	NAMES gmp
	DOC "gmp library"
)

set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
set(GMP_LIBRARIES ${GMP_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set GMP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gmp DEFAULT_MSG
	GMP_INCLUDE_DIR GMP_LIBRARY)
mark_as_advanced (GMP_INCLUDE_DIR GMP_LIBRARY)

