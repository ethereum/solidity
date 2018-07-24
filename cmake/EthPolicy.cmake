# it must be a macro cause policies have scopes
# http://www.cmake.org/cmake/help/v3.0/command/cmake_policy.html
macro (eth_policy)
	# link_directories() treats paths relative to the source dir.
	cmake_policy(SET CMP0015 NEW)

	if (${CMAKE_VERSION} VERSION_GREATER 3.0)

		# Avoid warnings in CMake 3.0.2:
		cmake_policy(SET CMP0042 NEW)
		cmake_policy(SET CMP0043 NEW)

		# allow VERSION argument in project()
		cmake_policy(SET CMP0048 NEW)

	endif()

	if (${CMAKE_VERSION} VERSION_GREATER 3.1)
		
		# do not interpret if() arguments as variables!
		cmake_policy(SET CMP0054 NEW)

	endif()

endmacro()

