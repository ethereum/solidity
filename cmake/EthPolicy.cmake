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
endmacro()
