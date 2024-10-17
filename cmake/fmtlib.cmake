find_package(fmt QUIET)
set(FMT_SYSTEM_HEADERS ON)

if(fmt_FOUND)
	message(STATUS "Using system-installed fmt library")
else()
	# Fallback to submodule if fmt is not found
	include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
	initialize_submodule(fmtlib)
	add_subdirectory(
		${CMAKE_SOURCE_DIR}/deps/fmtlib
		EXCLUDE_FROM_ALL
	)
endif()
