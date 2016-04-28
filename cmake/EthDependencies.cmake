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
	set (ETH_DEPENDENCY_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../extdep/install/windows/x64")
	set (CMAKE_PREFIX_PATH ${ETH_DEPENDENCY_INSTALL_DIR} ${CMAKE_PREFIX_PATH})

	# Qt5 requires opengl
	# TODO it windows SDK is NOT FOUND, throw ERROR
	# from https://github.com/rpavlik/cmake-modules/blob/master/FindWindowsSDK.cmake
	find_package(WINDOWSSDK REQUIRED)
	message(" - WindowsSDK dirs: ${WINDOWSSDK_DIRS}")
	set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${WINDOWSSDK_DIRS})
endif()

# homebrew install directories for few of our dependencies
set (CMAKE_PREFIX_PATH "/usr/local/opt/qt5" ${CMAKE_PREFIX_PATH})

# setup directory for cmake generated files and include it globally
# it's not used yet, but if we have more generated files, consider moving them to ETH_GENERATED_DIR
set(ETH_GENERATED_DIR "${PROJECT_BINARY_DIR}/gen")
include_directories(${ETH_GENERATED_DIR})

# boilerplate macros for some code editors
add_definitions(-DETH_TRUE)

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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

# TODO hanlde other msvc versions or it will fail find them
	set(Boost_COMPILER -vc120)
# use static boost libraries *.lib
	set(Boost_USE_STATIC_LIBS ON)

elseif (APPLE)

# use static boost libraries *.a
	set(Boost_USE_STATIC_LIBS ON)

elseif (UNIX)
# use dynamic boost libraries *.dll
	set(Boost_USE_STATIC_LIBS OFF)

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
		string(REPLACE "::" ";" MODULE_PARTS ${MODULE})
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
