# all dependencies that are not directly included in the cpp-ethereum distribution are defined here
# for this to work, download the dependency via the cmake script in extdep or install them manually!

function(eth_show_dependency DEP NAME)
	get_property(DISPLAYED GLOBAL PROPERTY ETH_${DEP}_DISPLAYED)
	if (NOT DISPLAYED)
		set_property(GLOBAL PROPERTY ETH_${DEP}_DISPLAYED TRUE)
		message(STATUS "${NAME} headers: ${${DEP}_INCLUDE_DIRS}")
		message(STATUS "${NAME} lib   : ${${DEP}_LIBRARIES}")
		if (NOT("${${DEP}_DLLS}" STREQUAL ""))
			message(STATUS "${NAME} dll   : ${${DEP}_DLLS}")
		endif()
	endif()
endfunction()

if (DEFINED MSVC)
	# by defining CMAKE_PREFIX_PATH variable, cmake will look for dependencies first in our own repository before looking in system paths like /usr/local/ ...
	# this must be set to point to the same directory as $ETH_DEPENDENCY_INSTALL_DIR in /extdep directory

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0)
		set (ETH_DEPENDENCY_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../extdep/install/windows/x64")
	else()
		get_filename_component(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/../deps/install" ABSOLUTE)
		set(ETH_DEPENDENCY_INSTALL_DIR
			"${DEPS_DIR}/x64"					# Old location for deps.
			"${DEPS_DIR}/win64"					# New location for deps.
			"${DEPS_DIR}/win64/Release/share"	# LLVM shared cmake files.
		)
	endif()
	set (CMAKE_PREFIX_PATH ${ETH_DEPENDENCY_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
endif()

# custom cmake scripts
set(ETH_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(ETH_SCRIPTS_DIR ${ETH_CMAKE_DIR}/scripts)

find_program(CTEST_COMMAND ctest)

#message(STATUS "CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}")
#message(STATUS "CMake Helper Path: ${ETH_CMAKE_DIR}")
#message(STATUS "CMake Script Path: ${ETH_SCRIPTS_DIR}")
#message(STATUS "ctest path: ${CTEST_COMMAND}")

## use multithreaded boost libraries, with -mt suffix
set(Boost_USE_MULTITHREADED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

# use static boost libraries *.lib
	set(Boost_USE_STATIC_LIBS ON)

elseif (APPLE)

# use static boost libraries *.a
	set(Boost_USE_STATIC_LIBS ON)

elseif (UNIX)
# use dynamic boost libraries *.dll
	set(Boost_USE_STATIC_LIBS OFF)

endif()

set(STATIC_LINKING FALSE CACHE BOOL "Build static binaries")

if (STATIC_LINKING)

	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)

	set(OpenSSL_USE_STATIC_LIBS ON)

	if (MSVC)
		# TODO - Why would we need .a on Windows?  Maybe some Cygwin-ism.
		# When I work through Windows static linkage, I will remove this,
		# if that is possible.
		set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	elseif (APPLE)
		# At the time of writing, we are still only PARTIALLY statically linked
		# on OS X, with a mixture of statically linked external libraries where
		# those are available, and dynamically linked where that is the only
		# option we have.    Ultimately, the aim would be for everything except
		# the runtime libraries to be statically linked.
		#
		# Still TODO:
		# - jsoncpp
		# - json-rpc-cpp
		# - leveldb (which pulls in snappy, for the dylib at ;east)
		# - miniupnp
		# - gmp
		#
		# Two further libraries (curl and zlib) ship as dylibs with the platform
		# but again we could build from source and statically link these too.
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a .dylib)
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
	endif()

	set(ETH_STATIC ON)
endif()

find_package(Boost 1.54.0 QUIET REQUIRED COMPONENTS thread date_time system regex chrono filesystem unit_test_framework program_options random)

eth_show_dependency(Boost boost)

if (APPLE)
	link_directories(/usr/local/lib)
	include_directories(/usr/local/include)
endif()

include_directories(BEFORE "${PROJECT_BINARY_DIR}/include")

function(eth_use TARGET REQUIRED)
	if (NOT TARGET ${TARGET})
		message(FATAL_ERROR "eth_use called for non existing target ${TARGET}")
	endif()

	if (TARGET ${PROJECT_NAME}_BuildInfo.h)
		add_dependencies(${TARGET} ${PROJECT_NAME}_BuildInfo.h)
	endif()

	foreach(MODULE ${ARGN})
		string(REPLACE "::" ";" MODULE_PARTS "${MODULE}")
		list(GET MODULE_PARTS 0 MODULE_MAIN)
		list(LENGTH MODULE_PARTS MODULE_LENGTH)
		if (MODULE_LENGTH GREATER 1)
			list(GET MODULE_PARTS 1 MODULE_SUB)
		endif()
		# TODO: check if file exists if not, throws FATAL_ERROR with detailed description
		get_target_property(TARGET_APPLIED ${TARGET} TARGET_APPLIED_${MODULE_MAIN}_${MODULE_SUB})
		if (NOT TARGET_APPLIED)
			include(Use${MODULE_MAIN})
			set_target_properties(${TARGET} PROPERTIES TARGET_APPLIED_${MODULE_MAIN}_${MODULE_SUB} TRUE)
			eth_apply(${TARGET} ${REQUIRED} ${MODULE_SUB})
		endif()
	endforeach()
endfunction()
