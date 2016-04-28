# Find libcpuid
#
# Find the libcpuid includes and library
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  CPUID_INCLUDE_DIRS, where to find header, etc.
#  CPUID_LIBRARIES, the libraries needed to use cpuid.
#  CPUID_FOUND, If false, do not try to use cpuid.

# only look in default directories
find_path(
	CPUID_INCLUDE_DIR
	NAMES libcpuid/libcpuid.h
	DOC "libcpuid include dir"
	)

find_library(
	CPUID_LIBRARY
	NAMES cpuid
	DOC "libcpuid library"
	)

set(CPUID_INCLUDE_DIRS ${CPUID_INCLUDE_DIR})
set(CPUID_LIBRARIES ${CPUID_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set CPUID_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cpuid DEFAULT_MSG CPUID_INCLUDE_DIR CPUID_LIBRARY)
mark_as_advanced (CPUID_INCLUDE_DIR CPUID_LIBRARY)

