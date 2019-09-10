function(create_build_info NAME)

	# Set build platform; to be written to BuildInfo.h
	set(ETH_BUILD_OS "${CMAKE_SYSTEM_NAME}")

	if (CMAKE_COMPILER_IS_MINGW)
		set(ETH_BUILD_COMPILER "mingw")
	elseif (CMAKE_COMPILER_IS_MSYS)
		set(ETH_BUILD_COMPILER "msys")
	elseif (CMAKE_COMPILER_IS_GNUCXX)
		set(ETH_BUILD_COMPILER "g++")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(ETH_BUILD_COMPILER "msvc")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(ETH_BUILD_COMPILER "clang")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(ETH_BUILD_COMPILER "appleclang")
	else ()
		set(ETH_BUILD_COMPILER "unknown")
	endif ()

	set(ETH_BUILD_PLATFORM "${ETH_BUILD_OS}.${ETH_BUILD_COMPILER}")

	#cmake build type may be not speCified when using msvc
	if (CMAKE_BUILD_TYPE)
		set(_cmake_build_type ${CMAKE_BUILD_TYPE})
	else()
		set(_cmake_build_type "${CMAKE_CFG_INTDIR}")
	endif()

	# Generate header file containing useful build information
	add_custom_target(${NAME}_BuildInfo.h ALL
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -DETH_SOURCE_DIR=${PROJECT_SOURCE_DIR} -DETH_BUILDINFO_IN=${ETH_CMAKE_DIR}/templates/BuildInfo.h.in -DETH_DST_DIR=${PROJECT_BINARY_DIR}/include/${PROJECT_NAME} -DETH_CMAKE_DIR=${ETH_CMAKE_DIR}
		-DETH_BUILD_TYPE="${_cmake_build_type}"
		-DETH_BUILD_OS="${ETH_BUILD_OS}"
		-DETH_BUILD_COMPILER="${ETH_BUILD_COMPILER}"
		-DETH_BUILD_PLATFORM="${ETH_BUILD_PLATFORM}"
		-DPROJECT_VERSION="${PROJECT_VERSION}"
		-DPROJECT_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
		-DPROJECT_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
		-DPROJECT_VERSION_PATCH="${PROJECT_VERSION_PATCH}"
		-P "${ETH_SCRIPTS_DIR}/buildinfo.cmake"
		)
	include_directories("${PROJECT_BINARY_DIR}/include")
endfunction()
