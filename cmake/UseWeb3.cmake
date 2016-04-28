function(eth_apply TARGET REQUIRED SUBMODULE)

	set(WEB3_DIR "${ETH_CMAKE_DIR}/../../webthree" CACHE PATH "The path to the webthree directory")
	set(WEB3_BUILD_DIR_NAME "build" CACHE STRING "The name of the build directory in web3")
	set(WEB3_BUILD_DIR "${WEB3_DIR}/${WEB3_BUILD_DIR_NAME}")
	set(CMAKE_LIBRARY_PATH ${WEB3_BUILD_DIR};${CMAKE_LIBRARY_PATH})
	
	find_package(Web3)
	target_include_directories(${TARGET} BEFORE PUBLIC ${Web3_INCLUDE_DIRS})

	if (${SUBMODULE} STREQUAL "whisper")
		eth_use(${TARGET} ${REQUIRED} Dev::devcore Dev::p2p Dev::devcrypto)
		target_link_libraries(${TARGET} ${Web3_WHISPER_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "webthree")
		eth_use(${TARGET} ${REQUIRED} Web3::whisper Eth::ethereum)
		target_link_libraries(${TARGET} ${Web3_WEBTHREE_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "web3jsonrpc")
		eth_use(${TARGET} ${REQUIRED} Mhd Web3::webthree JsonRpc::Client JsonRpc::Server)
		if (SOLIDITY)
			eth_use(${TARGET} ${REQUIRED} Solidity)
		endif (SOLIDITY)
		target_link_libraries(${TARGET} ${Web3_WEB3JSONRPC_LIBRARIES})
	endif()

endfunction()
