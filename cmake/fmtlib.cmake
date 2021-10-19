include(FetchContent)

FetchContent_Declare(
	fmtlib
	PREFIX "${CMAKE_BINARY_DIR}/deps"
	DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/downloads"
	DOWNLOAD_NAME fmt-8.0.1.tar.gz
	URL https://github.com/fmtlib/fmt/archive/8.0.1.tar.gz
	URL_HASH SHA256=b06ca3130158c625848f3fb7418f235155a4d389b2abc3a6245fb01cb0eb1e01
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
