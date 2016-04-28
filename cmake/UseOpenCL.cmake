function(eth_apply TARGET REQUIRED)
	find_package (OpenCL)
	eth_show_dependency(OpenCL OpenCL)
	if (OpenCL_FOUND)
		target_include_directories(${TARGET} SYSTEM PUBLIC ${OpenCL_INCLUDE_DIRS})
		if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux" AND ETHASHCL)
			# workaround for https://github.com/ethereum/alethzero/issues/69
			# force linking to libOpenCL as early as possible
			target_link_libraries(${TARGET} "-Wl,--no-as-needed -l${OpenCL_LIBRARIES} -Wl,--as-needed")
		else()
			target_link_libraries(${TARGET} ${OpenCL_LIBRARIES})
		endif()
		eth_copy_dlls(${TARGET} OpenCL_DLLS)
		# as per this comment: http://cmake.3232098.n2.nabble.com/Scope-of-find-package-inside-a-function-block-tp6821980p6822888.html
		# It seems that calling find_package() inside a cmake function will only set the found
		# and libs variables only for the local scope. We need to set it with a cached (global)
		# scope here in order to be visible outside the eth_use(${TARGET} ${REQUIRED} OpenCL) call
		set(OpenCL_FOUND 1 CACHE INTERNAL "")
	elseif (NOT ${REQUIRED} STREQUAL "OPTIONAL")
		message(FATAL_ERROR "OpenCL library not found")
	endif()
endfunction()
