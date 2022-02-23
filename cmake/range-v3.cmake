include(ExternalProject)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(RANGE_V3_CMAKE_COMMAND emcmake cmake)
else()
    set(RANGE_V3_CMAKE_COMMAND ${CMAKE_COMMAND})
endif()

set(prefix "${CMAKE_BINARY_DIR}/deps")
set(RANGE_V3_INCLUDE_DIR "${prefix}/include")

ExternalProject_Add(range-v3-project
    PREFIX "${prefix}"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/downloads"
    DOWNLOAD_NAME range-v3-0.11.0.tar.gz
    URL https://github.com/ericniebler/range-v3/archive/0.11.0.tar.gz
    URL_HASH SHA256=376376615dbba43d3bef75aa590931431ecb49eb36d07bb726a19f680c75e20c
    CMAKE_COMMAND ${RANGE_V3_CMAKE_COMMAND}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
               -DBUILD_TESTING=OFF
               -DRANGES_CXX_STD=${CMAKE_CXX_STANDARD}
               -DRANGE_V3_DOCS=OFF
               -DRANGE_V3_EXAMPLES=OFF
               -DRANGE_V3_TESTS=OFF
               -DRANGES_BUILD_CALENDAR_EXAMPLE=OFF
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    BUILD_BYPRODUCTS "${RANGE_V3_INCLUDE_DIR}/range/v3/all.hpp"
)

# Create range-v3 imported library
add_library(range-v3 INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${RANGE_V3_INCLUDE_DIR})  # Must exist.
set_target_properties(range-v3 PROPERTIES
    INTERFACE_COMPILE_OPTIONS "\$<\$<CXX_COMPILER_ID:MSVC>:/permissive->"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${RANGE_V3_INCLUDE_DIR}
    INTERFACE_INCLUDE_DIRECTORIES ${RANGE_V3_INCLUDE_DIR})
add_dependencies(range-v3 range-v3-project)
