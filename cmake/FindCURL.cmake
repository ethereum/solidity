# Find CURL
#
# Find the curl includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  CURL_INCLUDE_DIRS, where to find header, etc.
#  CURL_LIBRARIES, the libraries needed to use curl.
#  CURL_FOUND, If false, do not try to use curl.

# only look in default directories
find_path(
	CURL_INCLUDE_DIR 
	NAMES curl/curl.h
	DOC "curl include dir"
)

find_library(
	CURL_LIBRARY
	# names from cmake's FindCURL
	NAMES curl curllib libcurl_imp curllib_static libcurl
	DOC "curl library"
)

set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
set(CURL_LIBRARIES ${CURL_LIBRARY})

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	
	find_library(
		CURL_LIBRARY_DEBUG
		NAMES curld libcurld
		DOC "curl debug library"
	)
	
	set(CURL_LIBRARIES optimized ${CURL_LIBRARIES} debug ${CURL_LIBRARY_DEBUG})

	# prepare dlls
	string(REPLACE ".lib" ".dll" CURL_DLL ${CURL_LIBRARY})
	string(REPLACE "/lib/" "/bin/" CURL_DLL ${CURL_DLL})
	string(REPLACE ".lib" ".dll" CURL_DLL_DEBUG ${CURL_LIBRARY_DEBUG})
	string(REPLACE "/lib/" "/bin/" CURL_DLL_DEBUG ${CURL_DLL_DEBUG})
	set(CURL_DLLS optimized ${CURL_DLL} debug ${CURL_DLL_DEBUG})

endif()

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CURL DEFAULT_MSG
	CURL_INCLUDE_DIR CURL_LIBRARY)
mark_as_advanced (CURL_INCLUDE_DIR CURL_LIBRARY)

