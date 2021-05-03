include(FetchContent)

FetchContent_Declare(
	fmtlib
	PREFIX "${CMAKE_BINARY_DIR}/deps"
	DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/downloads"
	DOWNLOAD_NAME fmt-7.1.3.tar.gz
	URL https://github.com/fmtlib/fmt/archive/7.1.3.tar.gz
	URL_HASH SHA256=5cae7072042b3043e12d53d50ef404bbb76949dad1de368d7f993a15c8c05ecc
)

if (CMAKE_VERSION VERSION_LESS "3.14.0")
	FetchContent_GetProperties(fmtlib)
	if (NOT fmtlib_POPULATED)
		FetchContent_Populate(fmtlib)
		add_subdirectory(${fmtlib_SOURCE_DIR} ${fmtlib_BINARY_DIR})
	endif()
else()
	FetchContent_MakeAvailable(fmtlib)
endif()
