function(eth_apply TARGET REQUIRED SUBMODULE)

	set(SOL_DIR "${ETH_CMAKE_DIR}/.." CACHE PATH "The path to the solidity directory")
	set(SOL_BUILD_DIR_NAME  "build" CACHE STRING "The name of the build directory in solidity repo")
	set(SOL_BUILD_DIR "${SOL_DIR}/${SOL_BUILD_DIR_NAME}")
	set(CMAKE_LIBRARY_PATH ${SOL_BUILD_DIR};${CMAKE_LIBRARY_PATH})

	find_package(Solidity)

	# Hide confusing blank dependency information when using FindSolidity on itself.
	if (NOT(${MODULE_MAIN} STREQUAL Solidity))
		eth_show_dependency(SOLIDITY solidity)
	endif()

	target_include_directories(${TARGET} PUBLIC ${Solidity_INCLUDE_DIRS})

	if (${SUBMODULE} STREQUAL "solevmasm")
		target_link_libraries(${TARGET} ${Solidity_SOLEVMASM_LIBRARIES} jsoncpp)
	endif()

	if (${SUBMODULE} STREQUAL "lll")
		eth_use(${TARGET} ${REQUIRED} Solidity::solevmasm)
		target_link_libraries(${TARGET} ${Solidity_LLL_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "solidity" OR ${SUBMODULE} STREQUAL "")
		eth_use(${TARGET} ${REQUIRED} Dev::soldevcore Solidity::solevmasm)
		target_link_libraries(${TARGET} ${Solidity_SOLIDITY_LIBRARIES})
	endif()

	target_compile_definitions(${TARGET} PUBLIC ETH_SOLIDITY)
endfunction()
