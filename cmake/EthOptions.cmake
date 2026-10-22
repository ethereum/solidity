# CMAKE macros to set default CMAKE options and to show the
# resulting configuration.

macro(configure_project)
	set(NAME ${PROJECT_NAME})

	# features
	eth_default_option(COVERAGE OFF)

	# Master switch for the FuzzTest-based property tests. OFF means deps/fuzztest is never
	# add_subdirectory'd, so none of its transitive FetchContent deps (abseil/re2/gtest/antlr)
	# are configured.
	eth_default_option(PROPERTY_BASED_TESTS OFF)

	# Mode selector, only meaningful when PROPERTY_BASED_TESTS is ON.
	if (NOT DEFINED PROPERTY_BASED_TESTS_MODE)
		set(PROPERTY_BASED_TESTS_MODE "unittest" CACHE STRING "Property-based test mode: unittest or fuzzing")
	endif()
	set_property(CACHE PROPERTY_BASED_TESTS_MODE PROPERTY STRINGS "unittest" "fuzzing")

	# components
	eth_default_option(TESTS ON)
	eth_default_option(TOOLS ON)

	# Define a matching property name of each of the "features".
	foreach(FEATURE ${ARGN})
		set(SUPPORT_${FEATURE} TRUE)
	endforeach()

	include(EthBuildInfo)
	create_build_info(${NAME})
	print_config(${NAME})
endmacro()

macro(print_config NAME)
	message("")
	message("------------------------------------------------------------------------")
	message("-- Configuring ${NAME} ${PROJECT_VERSION}")
	message("------------------------------------------------------------------------")
	message("--                  CMake Version                            ${CMAKE_VERSION}")
	message("-- CMAKE_BUILD_TYPE Build type                               ${CMAKE_BUILD_TYPE}")
	message("-- TARGET_PLATFORM  Target platform                          ${CMAKE_SYSTEM_NAME}")
	message("--------------------------------------------------------------- features")
	message("-- COVERAGE         Coverage support                         ${COVERAGE}")
	message("------------------------------------------------------------- components")
if (SUPPORT_TESTS)
	message("-- TESTS            Build tests                              ${TESTS}")
endif()
if (SUPPORT_TOOLS)
	message("-- TOOLS            Build tools                              ${TOOLS}")
endif()
	message("------------------------------------------------------------------ flags")
	message("-- PROPERTY_BASED_TESTS (FuzzTest)                           ${PROPERTY_BASED_TESTS}")
	message("-- PROPERTY_BASED_TESTS_MODE                                 ${PROPERTY_BASED_TESTS_MODE}")
	message("------------------------------------------------------------------------")
	message("")
endmacro()
