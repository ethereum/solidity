include(ExternalProject)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	set(FMTLIB_CMAKE_COMMAND emcmake cmake)
else()
	set(FMTLIB_CMAKE_COMMAND ${CMAKE_COMMAND})
endif()

set(3rdparty_fmtlib_VERSION "7.1.3" CACHE STRING "fmtlib version")
set(3rdparty_fmtlib_CHECKSUM "SHA256=5cae7072042b3043e12d53d50ef404bbb76949dad1de368d7f993a15c8c05ecc" CACHE STRING "fmtlib checksum")
set(3rdparty_fmtlib_URL "https://github.com/fmtlib/fmt/archive/refs/tags/${3rdparty_fmtlib_VERSION}.tar.gz")

include(FetchContent)

FetchContent_Declare(
	fmtlib
	PREFIX "${CMAKE_BINARY_DIR}/deps"
	URL "${3rdparty_fmtlib_URL}"
	URL_HASH "${3rdparty_fmtlib_CHECKSUM}"
	DOWNLOAD_NAME "fmtlib-${3rdparty_fmtlib_VERSION}.tar.gz"
)

if(CMAKE_VERSION VERSION_LESS "3.14.0")
	FetchContent_GetProperties(fmtlib)
	if(NOT fmtlib_POPULATED)
	  FetchContent_Populate(fmtlib)
	  add_subdirectory(${fmtlib_SOURCE_DIR} ${fmtlib_BINARY_DIR})
	endif()
else()
	FetchContent_MakeAvailable(fmtlib)
endif()
