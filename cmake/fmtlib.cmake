include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
initialize_submodule(fmtlib)

set(FMT_SYSTEM_HEADERS ON)
add_subdirectory(
	${CMAKE_SOURCE_DIR}/deps/fmtlib
	EXCLUDE_FROM_ALL
)

