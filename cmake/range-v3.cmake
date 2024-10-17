find_package(range-v3 QUIET)

if(range-v3_FOUND)
	message(STATUS "Using system-installed range-v3 library")
else()
	include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
	initialize_submodule(range-v3)

	add_library(range-v3 INTERFACE IMPORTED)
	set_target_properties(range-v3 PROPERTIES
		INTERFACE_COMPILE_OPTIONS "\$<\$<CXX_COMPILER_ID:MSVC>:/permissive->"
		INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/range-v3/include
		INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/range-v3/include
	)
	add_dependencies(range-v3 range-v3-project)
endif()
