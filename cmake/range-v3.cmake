include(${CMAKE_SOURCE_DIR}/cmake/submodules.cmake)
initialize_submodule(range-v3)

add_library(range-v3 INTERFACE IMPORTED)
set_target_properties(range-v3 PROPERTIES
	INTERFACE_COMPILE_OPTIONS "\$<\$<CXX_COMPILER_ID:MSVC>:/permissive->"
	# Temporary workaround for libc++ 22+ (Apple Clang 21+), which no longer defines _LIBCPP_TEMPLATE_VIS used by
	# range-v3's std forward declarations. Those are only enabled on Apple Clang, so other platforms are unaffected.
	# TODO: Remove once range-v3 is updated past 0.12.0.
	INTERFACE_COMPILE_DEFINITIONS META_NO_STD_FORWARD_DECLARATIONS
	INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/range-v3/include
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/range-v3/include
)
add_dependencies(range-v3 range-v3-project)

