# it must be a macro cause policies have scopes
# http://www.cmake.org/cmake/help/v3.0/command/cmake_policy.html
macro (eth_policy)
	# link_directories() treats paths relative to the source dir.
	cmake_policy(SET CMP0015 NEW)

	# Avoid warnings in CMake 3.0.2:
	cmake_policy(SET CMP0042 NEW)
	cmake_policy(SET CMP0043 NEW)

	# allow VERSION argument in project()
	cmake_policy(SET CMP0048 NEW)

	if (POLICY CMP0054)
		# do not interpret if() arguments as variables!
		cmake_policy(SET CMP0054 NEW)
	endif()

	if (POLICY CMP0091)
		# Allow selecting MSVC runtime library using CMAKE_MSVC_RUNTIME_LIBRARY.
		cmake_policy(SET CMP0091 NEW)
	endif()

	# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
	if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
		cmake_policy(SET CMP0135 NEW)
	endif()

	if(POLICY CMP0115)
		# Require explicit extensions for source files, do not guess.
		# The extra calls to GetFileAttributesW significantly slow down cmake on Windows.
		# https://gitlab.kitware.com/cmake/cmake/-/issues/23154
		cmake_policy(SET CMP0115 NEW)
	endif()
endmacro()
