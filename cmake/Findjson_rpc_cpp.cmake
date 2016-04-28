# Find json-rcp-cpp 
#
# Find the json-rpc-cpp includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  JSON_RCP_CPP_INCLUDE_DIRS, where to find header, etc.
#  JSON_RCP_CPP_LIBRARIES, the libraries needed to use json-rpc-cpp.
#  JSON_RPC_CPP_SERVER_LIBRARIES, the libraries needed to use json-rpc-cpp-server
#  JSON_RPC_CPP_CLIENT_LIBRARIES, the libraries needed to use json-rpc-cpp-client
#  JSON_RCP_CPP_FOUND, If false, do not try to use json-rpc-cpp.
#  JSON_RPC_CPP_VERSION, version of library
#  JSON_RPC_CPP_VERSION_MAJOR
#  JSON_RPC_CPP_VERSION_MINOR
#  JSON_RPC_CPP_VERSION_PATCH


# only look in default directories
find_path(
	JSON_RPC_CPP_INCLUDE_DIR 
	NAMES jsonrpccpp/server.h jsonrpc/server.h
	PATH_SUFFIXES jsonrpc
	DOC "json-rpc-cpp include dir"
)

find_library(
	JSON_RPC_CPP_COMMON_LIBRARY
	NAMES jsonrpccpp-common
	DOC "json-rpc-cpp common library"
)

find_library(
	JSON_RPC_CPP_SERVER_LIBRARY
	NAMES jsonrpccpp-server
	DOC "json-rpc-cpp server library"
)

find_library(
	JSON_RPC_CPP_CLIENT_LIBRARY
	NAMES jsonrpccpp-client
	DOC "json-rpc-cpp client library"
)

# these are the variables to be uses by the calling script
set (JSON_RPC_CPP_INCLUDE_DIRS ${JSON_RPC_CPP_INCLUDE_DIR})
set (JSON_RPC_CPP_LIBRARIES ${JSON_RPC_CPP_COMMON_LIBRARY} ${JSON_RPC_CPP_SERVER_LIBRARY} ${JSON_RPC_CPP_CLIENT_LIBRARY})
set (JSON_RPC_CPP_SERVER_LIBRARIES ${JSON_RPC_CPP_COMMON_LIBRARY} ${JSON_RPC_CPP_SERVER_LIBRARY})
set (JSON_RPC_CPP_CLIENT_LIBRARIES ${JSON_RPC_CPP_COMMON_LIBRARY} ${JSON_RPC_CPP_CLIENT_LIBRARY})

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if (DEFINED MSVC)

	find_library(
		JSON_RPC_CPP_COMMON_LIBRARY_DEBUG
		NAMES jsonrpccpp-commond
		DOC "json-rpc-cpp common debug library"
	)
	
	find_library(
		JSON_RPC_CPP_SERVER_LIBRARY_DEBUG
		NAMES jsonrpccpp-serverd
		DOC "json-rpc-cpp server debug library"
	)

	find_library(
		JSON_RPC_CPP_CLIENT_LIBRARY_DEBUG
		NAMES jsonrpccpp-clientd
		DOC "json-rpc-cpp client debug library"
	)

	set (JSON_RPC_CPP_LIBRARIES
		optimized ${JSON_RPC_CPP_COMMON_LIBRARY}
		optimized ${JSON_RPC_CPP_SERVER_LIBRARY}
		optimized ${JSON_RPC_CPP_CLIENT_LIBRARY}
		debug ${JSON_RPC_CPP_COMMON_LIBRARY_DEBUG}
		debug ${JSON_RPC_CPP_SERVER_LIBRARY_DEBUG}
		debug ${JSON_RPC_CPP_CLIENT_LIBRARY_DEBUG}
	)
	
	set (JSON_RPC_CPP_SERVER_LIBRARIES
		optimized ${JSON_RPC_CPP_COMMON_LIBRARY}
		optimized ${JSON_RPC_CPP_SERVER_LIBRARY}
		debug ${JSON_RPC_CPP_COMMON_LIBRARY_DEBUG}
		debug ${JSON_RPC_CPP_SERVER_LIBRARY_DEBUG}
	)

	set (JSON_RPC_CPP_CLIENT_LIBRARIES
		optimized ${JSON_RPC_CPP_COMMON_LIBRARY}
		optimized ${JSON_RPC_CPP_CLIENT_LIBRARY}
		debug ${JSON_RPC_CPP_COMMON_LIBRARY_DEBUG}
		debug ${JSON_RPC_CPP_CLIENT_LIBRARY_DEBUG}
	)

endif()

if (JSON_RPC_CPP_INCLUDE_DIR)
	set (JSON_RPC_CPP_VERSION_HEADER "${JSON_RPC_CPP_INCLUDE_DIR}/jsonrpccpp/version.h")
	if (EXISTS ${JSON_RPC_CPP_VERSION_HEADER})
		file (STRINGS ${JSON_RPC_CPP_VERSION_HEADER} JSON_RPC_CPP_VERSION_MAJOR REGEX "^#define JSONRPC_CPP_MAJOR_VERSION[ \t]+[0-9]+$") 
		file (STRINGS ${JSON_RPC_CPP_VERSION_HEADER} JSON_RPC_CPP_VERSION_MINOR REGEX "^#define JSONRPC_CPP_MINOR_VERSION[ \t]+[0-9]+$") 
		file (STRINGS ${JSON_RPC_CPP_VERSION_HEADER} JSON_RPC_CPP_VERSION_PATCH REGEX "^#define JSONRPC_CPP_PATCH_VERSION[ \t]+[0-9]+$") 
		string (REGEX REPLACE "^#define JSONRPC_CPP_MAJOR_VERSION[ \t]+([0-9]+)" "\\1" JSON_RPC_CPP_VERSION_MAJOR ${JSON_RPC_CPP_VERSION_MAJOR})
		string (REGEX REPLACE "^#define JSONRPC_CPP_MINOR_VERSION[ \t]+([0-9]+)" "\\1" JSON_RPC_CPP_VERSION_MINOR ${JSON_RPC_CPP_VERSION_MINOR})
		string (REGEX REPLACE "^#define JSONRPC_CPP_PATCH_VERSION[ \t]+([0-9]+)" "\\1" JSON_RPC_CPP_VERSION_PATCH ${JSON_RPC_CPP_VERSION_PATCH})
		set (JSON_RPC_CPP_VERSION ${JSON_RPC_CPP_VERSION_MAJOR}.${JSON_RPC_CPP_VERSION_MINOR}.${JSON_RPC_CPP_VERSION_PATCH})
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set JSON_RPC_CPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	json_rpc_cpp
	REQUIRED_VARS JSON_RPC_CPP_INCLUDE_DIR JSON_RPC_CPP_COMMON_LIBRARY JSON_RPC_CPP_SERVER_LIBRARY JSON_RPC_CPP_CLIENT_LIBRARY
	VERSION_VAR JSON_RPC_CPP_VERSION
)

mark_as_advanced (JSON_RPC_CPP_INCLUDE_DIR JSON_RPC_CPP_COMMON_LIBRARY JSON_RPC_CPP_SERVER_LIBRARY JSON_RPC_CPP_CLIENT_LIBRARY)

