# Property-based / fuzz tests built on Google FuzzTest (deps/fuzztest).
#
# This whole directory is only added when PROPERTY_BASED_TESTS is ON (see ../CMakeLists.txt),
# so the FuzzTest framework and its transitive FetchContent dependencies (abseil/re2/gtest/
# antlr) are not configured in normal builds.

# Translate our own mode option into FuzzTest's internal flag. FuzzTest reads
# FUZZTEST_FUZZING_MODE at configure time via option(); setting a normal variable of the
# same name here wins under policy CMP0077 (NEW). GCC + fuzzing mode is rejected by
# FuzzTest's own FATAL_ERROR, so we do not duplicate that guard.
if (PROPERTY_BASED_TESTS_MODE STREQUAL "fuzzing")
	set(FUZZTEST_FUZZING_MODE ON)
elseif (PROPERTY_BASED_TESTS_MODE STREQUAL "unittest")
	set(FUZZTEST_FUZZING_MODE OFF)
else()
	message(FATAL_ERROR "PROPERTY_BASED_TESTS_MODE must be 'unittest' or 'fuzzing', got '${PROPERTY_BASED_TESTS_MODE}'")
endif()

# Silence warnings for the FuzzTest subtree. The project adds -Werror (and -Wconversion etc.)
# globally via add_compile_options before this directory is created, and those flags are
# inherited here; FuzzTest and its dependencies do not build cleanly under them. Appended
# after the inherited flags, -w wins.
add_compile_options(-w)

include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
initialize_submodule(fuzztest)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/fuzztest ${CMAKE_BINARY_DIR}/deps/fuzztest EXCLUDE_FROM_ALL)

# Adds coverage instrumentation flags to targets defined below when in fuzzing mode; no-op in unit-test mode.
fuzztest_setup_fuzzing_flags()

set(yul_fuzztest_sources
	libyul/SSACFGInterpreter.cpp
	libyul/SSACFGInterpreter.h
	libyul/TarjanSCCPropertyTest.cpp
)
detect_stray_source_files("${yul_fuzztest_sources}" "libyul/")

add_executable(yul_fuzztest ${yul_fuzztest_sources})
target_link_libraries(yul_fuzztest PRIVATE yul)
link_fuzztest(yul_fuzztest)

set(evmasm_fuzztest_sources
	libevmasm/BlockDeduplicatorPropertyTest.cpp
)
detect_stray_source_files("${evmasm_fuzztest_sources}" "libevmasm/")

add_executable(evmasm_fuzztest ${evmasm_fuzztest_sources})
target_link_libraries(evmasm_fuzztest PRIVATE evmasm)
link_fuzztest(evmasm_fuzztest)

# Convenience target to build all per-library property-test executables at once (e.g. in CI).
# New <lib>_fuzztest targets should add themselves here.
add_custom_target(property_tests)
add_dependencies(property_tests yul_fuzztest evmasm_fuzztest)
