include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/submodules.cmake)
initialize_submodule(nlohmann-json)

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(
	${CMAKE_CURRENT_SOURCE_DIR}/deps/nlohmann-json
	EXCLUDE_FROM_ALL
)

