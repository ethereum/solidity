cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0015 NEW)
# let cmake autolink dependencies on windows
cmake_policy(SET CMP0020 NEW)
# this policy was introduced in cmake 3.0
# remove if, once 3.0 will be used on unix
if (${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0043 OLD)
endif()

# TODO use version from Version.h
set(PROJECT_VERSION "0.9.42")
if (${CMAKE_VERSION} VERSION_GREATER 3.0)
	cmake_policy(SET CMP0042 OLD) 	# fix MACOSX_RPATH
	cmake_policy(SET CMP0048 NEW) 	# allow VERSION argument in project()
	project(solidity VERSION ${PROJECT_VERSION})
else()
	project(solidity)
endif()

# Figure out environment.
set(CPPETHEREUM_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-ethereum/${BUILD_DIR_NAME}" CACHE STRING "The absolute build directory of cpp-ethereum.")
set(ETH_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/../cpp-ethereum-cmake"   CACHE PATH "The the path to the cmake directory")
set(ETH_DIR         "${CMAKE_CURRENT_LIST_DIR}/../cpp-ethereum"         CACHE PATH "The path to the cpp-ethereum directory")
set(BUILD_DIR_NAME  "build"                                            CACHE STRING "The name of the build directory in cpp-ethereum")
set(ETH_BUILD_DIR   "${ETH_DIR}/${BUILD_DIR_NAME}")

list(APPEND CMAKE_MODULE_PATH ${ETH_CMAKE_DIR})

include(EthDependencies)
include(EthExecutableHelper)
include(EthCompilerSettings)

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(libsolidity)
# Include a directory with BuildInfo.h
include_directories(${ETH_BUILD_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../cpp-ethereum)
include_directories(${CPPETHEREUM_BUILD})

file(GLOB_RECURSE SRC_LIST "libsolidity/*.cpp")
file(GLOB_RECURSE HEADERS "libsolidity/*.h")

set(EXECUTABLE solidity)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

# TODO installation and packaging rules
