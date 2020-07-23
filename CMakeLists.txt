cmake_minimum_required(VERSION 3.9.0)

set(ETH_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/cmake"   CACHE PATH "The the path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${ETH_CMAKE_DIR})

include(EthToolchains)

# Set cmake_policies
include(EthPolicy)
eth_policy()

# project name and version should be set after cmake_policy CMP0048
set(PROJECT_VERSION "0.7.0")
# OSX target needed in order to support std::visit
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
project(solidity VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if (IS_BIG_ENDIAN)
	message(FATAL_ERROR "${PROJECT_NAME} currently does not support big endian systems.")
endif()

option(SOLC_LINK_STATIC "Link solc executable statically on supported platforms" OFF)

# Setup cccache.
include(EthCcache)

# Let's find our dependencies
include(EthDependencies)
include(jsoncpp)
include_directories(SYSTEM ${JSONCPP_INCLUDE_DIR})

find_package(Threads)

# Figure out what compiler and system are we using
include(EthCompilerSettings)

# Include utils
include(EthUtils)

# Create license.h from LICENSE.txt and template
# Converting to char array is required due to MSVC's string size limit.
file(READ ${CMAKE_SOURCE_DIR}/LICENSE.txt LICENSE_TEXT HEX)
string(REGEX MATCHALL ".." LICENSE_TEXT "${LICENSE_TEXT}")
string(REGEX REPLACE ";" ",\n\t0x" LICENSE_TEXT "${LICENSE_TEXT}")
set(LICENSE_TEXT "0x${LICENSE_TEXT}")

configure_file("${CMAKE_SOURCE_DIR}/cmake/templates/license.h.in" include/license.h)

include(EthOptions)
configure_project(TESTS)

find_package(Z3 4.6.0)
if (${Z3_FOUND})
  add_definitions(-DHAVE_Z3)
  message("Z3 SMT solver found. This enables optional SMT checking with Z3.")
endif()

find_package(CVC4 QUIET)
if (${CVC4_FOUND})
  add_definitions(-DHAVE_CVC4)
  message("CVC4 SMT solver found. This enables optional SMT checking with CVC4.")
endif()

if (NOT (${Z3_FOUND} OR ${CVC4_FOUND}))
  message("No SMT solver found (or it has been forcefully disabled). Optional SMT checking will not be available.\
  \nPlease install Z3 or CVC4 or remove the option disabling them (USE_Z3, USE_CVC4).")
endif()

add_subdirectory(libsolutil)
add_subdirectory(liblangutil)
add_subdirectory(libsmtutil)
add_subdirectory(libevmasm)
add_subdirectory(libyul)
add_subdirectory(libsolidity)
add_subdirectory(libsolc)
add_subdirectory(tools)

if (NOT EMSCRIPTEN)
	add_subdirectory(solc)
endif()

if (TESTS AND NOT EMSCRIPTEN)
	add_subdirectory(test)
endif()
