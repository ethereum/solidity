macro(configure_project)
	set(NAME ${PROJECT_NAME})

	# features
	eth_default_option(VMTRACE OFF)
	eth_default_option(PROFILING OFF)
	eth_default_option(FATDB ON)
	eth_default_option(ROCKSDB OFF)
	eth_default_option(OLYMPIC OFF)
	eth_default_option(PARANOID OFF)
	eth_default_option(MINIUPNPC ON)

	# components
	eth_default_option(GUI ON)
	eth_default_option(TESTS ON)
	eth_default_option(TOOLS ON)
	eth_default_option(ETHASHCL ON)
	eth_default_option(EVMJIT ON)
	eth_default_option(SOLIDITY ON)

	# Resolve any clashes between incompatible options.
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		if (PARANOID)
			message("Paranoia requires debug - disabling for release build.")
			set(PARANOID OFF)
		endif ()
		if (VMTRACE)
			message("VM Tracing requires debug - disabling for release build.")
			set (VMTRACE OFF)
		endif ()
	endif ()

	# Define a matching property name of each of the "features".
	foreach(FEATURE ${ARGN})
		set(SUPPORT_${FEATURE} TRUE)
	endforeach()

	# TODO:  Eliminate this pre-processor symbol, which is a bad pattern.
	# Common code has no business knowing which application it is part of.
	add_definitions(-DETH_TRUE)

	# TODO:  Why do we even bother with Olympic support anymore?
	if (OLYMPIC)
		add_definitions(-DETH_OLYMPIC)
	else()
		add_definitions(-DETH_FRONTIER)
	endif()

	# Are we including the JIT EVM module?
	# That pulls in a quite heavyweight LLVM dependency, which is
	# not suitable for all platforms.
	if (EVMJIT)
		add_definitions(-DETH_EVMJIT)
	endif ()

	# FATDB is an option to include the reverse hashes for the trie,
	# i.e. it allows you to iterate over the contents of the state.
	if (FATDB)
		add_definitions(-DETH_FATDB)
	endif ()

	# TODO:  What does "paranoia" even mean?
	if (PARANOID)
		add_definitions(-DETH_PARANOIA)
	endif ()

	# TODO:  What does "VM trace" even mean?
	if (VMTRACE)
		add_definitions(-DETH_VMTRACE)
	endif ()

	# CI Builds should provide (for user builds this is totally optional)
	# -DBUILD_NUMBER - A number to identify the current build with. Becomes TWEAK component of project version.
	# -DVERSION_SUFFIX - A string to append to the end of the version string where applicable.
	if (NOT DEFINED BUILD_NUMBER)
		# default is big so that local build is always considered greater
		# and can easily replace CI build for for all platforms if needed.
		# Windows max version component number is 65535
		set(BUILD_NUMBER 65535)
	endif()

	# Suffix like "-rc1" e.t.c. to append to versions wherever needed.
	if (NOT DEFINED VERSION_SUFFIX)
		set(VERSION_SUFFIX "")
	endif()

	include(EthBuildInfo)
	create_build_info(${NAME})
	print_config(${NAME})
endmacro()

macro(print_config NAME)
	message("")
	message("------------------------------------------------------------------------")
	message("-- Configuring ${NAME}")
	message("------------------------------------------------------------------------")
	message("--                  CMake Version                            ${CMAKE_VERSION}")
	message("-- CMAKE_BUILD_TYPE Build type                               ${CMAKE_BUILD_TYPE}")
	message("-- TARGET_PLATFORM  Target platform                          ${CMAKE_SYSTEM_NAME}")
	message("--------------------------------------------------------------- features")
if (SUPPORT_CPUID)
	message("--                  Hardware identification support          ${CPUID_FOUND}")
endif()
if (SUPPORT_CURL)
	message("--                  HTTP Request support                     ${CURL_FOUND}")
endif()
if (SUPPORT_VMTRACE)
	message("-- VMTRACE          VM execution tracing                     ${VMTRACE}")
endif()
	message("-- PROFILING        Profiling support                        ${PROFILING}")
if (SUPPORT_FATDB)
	message("-- FATDB            Full database exploring                  ${FATDB}")
endif()
if (SUPPORT_ROCKSDB)
	message("-- ROCKSDB          Prefer rocksdb to leveldb                ${ROCKSDB}")
endif()
if (SUPPORT_OLYMPIC)
	message("-- OLYMPIC          Default to the Olympic network           ${OLYMPIC}")
endif()
if (SUPPORT_PARANOID)
	message("-- PARANOID         -                                        ${PARANOID}")
endif()
if (SUPPORT_MINIUPNPC)
	message("-- MINIUPNPC        -                                        ${MINIUPNPC}")
endif()
	message("------------------------------------------------------------- components")
if (SUPPORT_GUI)
	message("-- GUI              Build GUI components                     ${GUI}")
endif()
if (SUPPORT_TESTS)
	message("-- TESTS            Build tests                              ${TESTS}")
endif()
if (SUPPORT_TOOLS)
	message("-- TOOLS            Build tools                              ${TOOLS}")
endif()
if (SUPPORT_ETHASHCL)
	message("-- ETHASHCL         Build OpenCL components                  ${ETHASHCL}")
endif()
if (SUPPORT_EVMJIT)
	message("-- EVMJIT           Build LLVM-based JIT EVM                 ${EVMJIT}")
endif()
if (SUPPORT_SOLIDITY)
	message("-- SOLIDITY         Build Solidity                           ${SOLIDITY}")
endif()
	message("------------------------------------------------------------------------")
	message("")
endmacro()
