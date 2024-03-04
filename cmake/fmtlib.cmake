include(FetchContent)

FetchContent_Declare(
	fmtlib
	PREFIX "${PROJECT_BINARY_DIR}/deps"
	DOWNLOAD_DIR "${PROJECT_SOURCE_DIR}/deps/downloads"
	DOWNLOAD_NAME fmt-9.1.0.tar.gz
	URL https://github.com/fmtlib/fmt/archive/9.1.0.tar.gz
	URL_HASH SHA256=5dea48d1fcddc3ec571ce2058e13910a0d4a6bab4cc09a809d8b1dd1c88ae6f2
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
