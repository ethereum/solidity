function(eth_apply TARGET REQUIRED SUBMODULE)

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
		target_link_libraries(${TARGET} devcore)
	endif()

endfunction()
