function(eth_apply TARGET REQUIRED SUBMODULE)

	set(DEV_DIR "${ETH_CMAKE_DIR}/../../libweb3core" CACHE PATH "The path to dev libraries directory")
	set(DEV_BUILD_DIR_NAME "build" CACHE STRING "Dev build directory name")
	set(DEV_BUILD_DIR "${DEV_DIR}/${DEV_BUILD_DIR_NAME}")
	set(CMAKE_LIBRARY_PATH ${DEV_BUILD_DIR} ${CMAKE_LIBRARY_PATH})

	find_package(Dev)

	target_include_directories(${TARGET} BEFORE PUBLIC ${Dev_INCLUDE_DIRS})

	# Base is where all dependencies for devcore are
	if (${SUBMODULE} STREQUAL "base")
		# if it's ethereum source dir, alwasy build BuildInfo.h before
		eth_use(${TARGET} ${REQUIRED} Dev::buildinfo Jsoncpp)
		if (NOT EMSCRIPTEN)
			eth_use(${TARGET} ${REQUIRED} DB::auto)
		endif()
		target_include_directories(${TARGET} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})

		target_link_libraries(${TARGET} ${Boost_THREAD_LIBRARIES})
		target_link_libraries(${TARGET} ${Boost_RANDOM_LIBRARIES})
		target_link_libraries(${TARGET} ${Boost_FILESYSTEM_LIBRARIES})
		target_link_libraries(${TARGET} ${Boost_SYSTEM_LIBRARIES})

		if (DEFINED MSVC)
			target_link_libraries(${TARGET} ${Boost_CHRONO_LIBRARIES})
			target_link_libraries(${TARGET} ${Boost_DATE_TIME_LIBRARIES})
		endif()

		if (NOT DEFINED MSVC)
			target_link_libraries(${TARGET} pthread)
		endif()
	endif()

	if (${SUBMODULE} STREQUAL "devcore")
		eth_use(${TARGET} ${REQUIRED} Dev::base)
		target_link_libraries(${TARGET} ${Dev_DEVCORE_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "devcrypto")
		eth_use(${TARGET} ${REQUIRED} Dev::devcore Utils::scrypt Cryptopp)
		if ((NOT EMSCRIPTEN) AND (NOT DEFINED MSVC))
			eth_use(${TARGET} ${REQUIRED} Utils::secp256k1)
		endif()

		target_link_libraries(${TARGET} ${Dev_DEVCRYPTO_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "p2p")
		eth_use(${TARGET} ${REQUIRED} Dev::devcore Dev::devcrypto)
		eth_use(${TARGET} OPTIONAL Miniupnpc)
		target_link_libraries(${TARGET} ${Dev_P2P_LIBRARIES})
	endif()

endfunction()
