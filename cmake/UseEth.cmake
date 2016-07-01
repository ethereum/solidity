function(eth_apply TARGET REQUIRED SUBMODULE)
	# TODO take into account REQUIRED

	set(ETH_DIR "${ETH_CMAKE_DIR}/../../libethereum" CACHE PATH "The path to the ethereum directory")
	set(ETH_BUILD_DIR_NAME  "build" CACHE STRING "Ethereum build directory name")
	set(ETH_BUILD_DIR "${ETH_DIR}/${ETH_BUILD_DIR_NAME}")
	set(CMAKE_LIBRARY_PATH 	${ETH_BUILD_DIR};${CMAKE_LIBRARY_PATH})

	find_package(Eth)

	target_include_directories(${TARGET} BEFORE PUBLIC ${Eth_INCLUDE_DIRS})
	if ((DEFINED cpp-ethereum_VERSION) OR (DEFINED ethereum_VERSION))
		target_include_directories(${TARGET} PUBLIC "${CMAKE_BINARY_DIR}/libethereum/include/")
	else()
		target_include_directories(${TARGET} PUBLIC "${ETH_BUILD_DIR}/include/")
	endif()

	if (${SUBMODULE} STREQUAL "ethash")
		# even if ethash is required, Cryptopp is optional
		eth_use(${TARGET} OPTIONAL Cryptopp)
		target_link_libraries(${TARGET} ${Eth_ETHASH_LIBRARIES})
		# even if ethash is required, ethash-cl and cpuid are optional

		# workaround for https://github.com/ethereum/alethzero/issues/69
		# force linking to libOpenCL as early as possible
		if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux" AND ETHASHCL AND GUI)
			find_package (OpenCL)
			if (OpenCL_FOUND)
				target_link_libraries(${TARGET} "-Wl,--no-as-needed -l${OpenCL_LIBRARIES} -Wl,--as-needed")
			endif()
		endif()
	endif()

	if (${SUBMODULE} STREQUAL "ethash-cl")
		if (ETHASHCL)
			eth_use(${TARGET} ${REQUIRED} OpenCL)
			if (OpenCL_FOUND)
				eth_use(${TARGET} ${REQUIRED} Eth::ethash)
				target_include_directories(${TARGET} SYSTEM PUBLIC ${OpenCL_INCLUDE_DIRS})
				target_link_libraries(${TARGET} ${Eth_ETHASH-CL_LIBRARIES})
				target_compile_definitions(${TARGET} PUBLIC ETH_ETHASHCL)
			endif()
		endif()
	endif()

	if (${SUBMODULE} STREQUAL "ethcore")
		eth_use(${TARGET} ${REQUIRED} Dev::devcrypto Dev::buildinfo Dev::devcore)
		target_link_libraries(${TARGET} ${Eth_ETHCORE_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "evmcore")
		eth_use(${TARGET} ${REQUIRED} Dev::devcore)
		target_link_libraries(${TARGET} ${Eth_EVMCORE_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "evmjit")
		# TODO: not sure if should use evmjit
		# TODO: take into account REQUIRED variable
		if (EVMJIT)
			target_link_libraries(${TARGET} ${Eth_EVMJIT_LIBRARIES})
			eth_copy_dlls(${TARGET} EVMJIT_DLLS)
		endif()
	endif()

	if (${SUBMODULE} STREQUAL "evm")
		eth_use(${TARGET} ${REQUIRED} Eth::ethcore Dev::devcrypto Eth::evmcore Dev::devcore)
		eth_use(${TARGET} OPTIONAL Eth::evmjit)
		target_link_libraries(${TARGET} ${Eth_EVM_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "ethashseal")
		eth_use(${TARGET} ${REQUIRED} Eth::ethereum Eth::ethash)
		eth_use(${TARGET} OPTIONAL Eth::ethash-cl Cpuid)
		target_link_libraries(${TARGET} ${Eth_ETHASHSEAL_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "ethereum")
		eth_use(${TARGET} ${REQUIRED} Eth::evm Eth::ethcore)
		if (NOT EMSCRIPTEN)
			eth_use(${TARGET} ${REQUIRED} Dev::p2p Dev::devcrypto)
		endif()
		target_link_libraries(${TARGET} ${Boost_REGEX_LIBRARIES})
		target_link_libraries(${TARGET} ${Eth_ETHEREUM_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "natspec")
		target_link_libraries(${TARGET} ${Eth_NATSPEC_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "testutils")
		eth_use(${TARGET} ${REQUIRED} Eth::ethereum)
		target_link_libraries(${TARGET} ${Eth_TESTUTILS_LIBRARIES})
	endif()

endfunction()
