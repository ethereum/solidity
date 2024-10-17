find_package(nlohmann_json QUIET)

set(JSON_Install OFF CACHE INTERNAL "")

if(nlohmann_json_FOUND)
	message(STATUS "Using system-installed nlohmann-json library")
else()
	include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
	initialize_submodule(nlohmann-json)
	add_subdirectory(
		${CMAKE_SOURCE_DIR}/deps/nlohmann-json
		EXCLUDE_FROM_ALL
	)
endif()
