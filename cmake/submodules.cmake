macro(initialize_submodule SUBMODULE_PATH)
	if(NOT IGNORE_VENDORED_DEPENDENCIES)
		file(GLOB submodule_contents "${CMAKE_SOURCE_DIR}/deps/${SUBMODULE_PATH}/*")

		if(submodule_contents)
			message(STATUS "git submodule '${SUBMODULE_PATH}' seem to be already initialized: nothing to do.")
		else()
			message(STATUS "git submodule '${SUBMODULE_PATH}' seem not to be initialized: implicitly executing 'git submodule update --init '${CMAKE_SOURCE_DIR}/deps/${SUBMODULE_PATH}'.")
			find_package(Git)
			if(NOT Git_FOUND)
				message(FATAL_ERROR "Failed to initialize submodules: 'git' command not found.")
			endif()
			execute_process(
					COMMAND git submodule update --init ${CMAKE_SOURCE_DIR}/deps/${SUBMODULE_PATH}
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
					RESULT_VARIABLE result
			)
			if(NOT result EQUAL 0)
				message(FATAL_ERROR "Failed to initialize submodules: 'git submodule update --init' failed.")
			endif()
		endif()
	endif()
endmacro()
