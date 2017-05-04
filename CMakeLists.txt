cmake_minimum_required(VERSION 3.0.0)

set(ETH_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/cmake"   CACHE PATH "The the path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${ETH_CMAKE_DIR})

# Set cmake_policies
include(EthPolicy)
eth_policy()

# project name and version should be set after cmake_policy CMP0048
set(PROJECT_VERSION "0.4.12")
project(solidity VERSION ${PROJECT_VERSION})

# Let's find our dependencies
include(EthDependencies)
include(deps/jsoncpp.cmake)

# Figure out what compiler and system are we using
include(EthCompilerSettings)

# Include helper macros
include(EthExecutableHelper)

# Include utils
include(EthUtils)

include(EthOptions)
configure_project(TESTS)

add_subdirectory(libdevcore)
add_subdirectory(libevmasm)
add_subdirectory(libsolidity)
add_subdirectory(solc)

if (NOT EMSCRIPTEN)
	add_subdirectory(liblll)
	add_subdirectory(lllc)
endif()

if (TESTS AND NOT EMSCRIPTEN)
	add_subdirectory(test)
endif()
