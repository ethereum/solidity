include(ExternalProject)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(JSONCPP_CMAKE_COMMAND emcmake cmake)
else()
    set(JSONCPP_CMAKE_COMMAND ${CMAKE_COMMAND})
endif()

# Disable implicit fallthrough warning in jsoncpp for gcc >= 7 until the upstream handles it properly
if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
    set(JSONCCP_EXTRA_FLAGS -Wno-implicit-fallthrough)
else()
    set(JSONCCP_EXTRA_FLAGS "")
endif()

set(prefix "${CMAKE_BINARY_DIR}/deps")
set(JSONCPP_LIBRARY "${prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jsoncpp${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(JSONCPP_INCLUDE_DIR "${prefix}/include")

set(byproducts "")
if(CMAKE_VERSION VERSION_GREATER 3.1)
    set(byproducts BUILD_BYPRODUCTS "${JSONCPP_LIBRARY}")
endif()

ExternalProject_Add(jsoncpp-project
    PREFIX "${prefix}"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/downloads"
    DOWNLOAD_NAME jsoncpp-1.7.7.tar.gz
    URL https://github.com/open-source-parsers/jsoncpp/archive/1.7.7.tar.gz
    URL_HASH SHA256=087640ebcf7fbcfe8e2717a0b9528fff89c52fcf69fa2a18cc2b538008098f97
    CMAKE_COMMAND ${JSONCPP_CMAKE_COMMAND}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
               # Build static lib but suitable to be included in a shared lib.
               -DCMAKE_POSITION_INDEPENDENT_CODE=${BUILD_SHARED_LIBS}
               -DJSONCPP_WITH_TESTS=OFF
               -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF
               -DCMAKE_CXX_FLAGS=${JSONCCP_EXTRA_FLAGS}
    # Overwrite build and install commands to force Release build on MSVC.
    BUILD_COMMAND cmake --build <BINARY_DIR> --config Release
    INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target install
    ${byproducts}
)

# Create jsoncpp imported library
add_library(jsoncpp STATIC IMPORTED)
file(MAKE_DIRECTORY ${JSONCPP_INCLUDE_DIR})  # Must exist.
set_property(TARGET jsoncpp PROPERTY IMPORTED_LOCATION ${JSONCPP_LIBRARY})
set_property(TARGET jsoncpp PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${JSONCPP_INCLUDE_DIR})
add_dependencies(jsoncpp jsoncpp-project)
