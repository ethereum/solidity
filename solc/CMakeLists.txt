set(libsolcli_sources
	CommandLineInterface.cpp CommandLineInterface.h
	CommandLineParser.cpp CommandLineParser.h
)

set(libsolcli_libs solidity Boost::boost Boost::program_options)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	# Since we do not want to depend on networking code on production binaries,
	# this feature is only available when creating a debug build.
	# the LSP's TCP listener is exclusively used more convenient debugging.
	option(SOLC_LSP_TCP "Solidity LSP: Enables TCP listener support (should only be eanbled for debugging purposes)." OFF)
	if(SOLC_LSP_TCP)
		set(libsolcli_defines SOLC_LSP_TCP=1)
		list(APPEND libsolcli_sources LSPTCPTransport.cpp LSPTCPTransport.h)
		find_package(Threads REQUIRED)
		list(APPEND libsolcli_libs Threads::Threads)
	endif()
endif()

add_library(solcli ${libsolcli_sources})
target_compile_definitions(solcli PUBLIC ${libsolcli_defines})
target_link_libraries(solcli PUBLIC ${libsolcli_libs})

set(sources main.cpp)

add_executable(solc ${sources})
target_link_libraries(solc PRIVATE solcli)

include(GNUInstallDirs)
install(TARGETS solc DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(SOLC_LINK_STATIC AND UNIX AND NOT APPLE)
	# Produce solc as statically linked binary (includes C/C++ standard libraries)
	# This is not supported on macOS, see
	# https://developer.apple.com/library/content/qa/qa1118/_index.html.
	set_target_properties(
		solc PROPERTIES
		LINK_FLAGS -static
		LINK_SEARCH_START_STATIC ON
		LINK_SEARCH_END_STATIC ON
	)
elseif(SOLC_STATIC_STDLIBS AND UNIX AND NOT APPLE)
	set_target_properties(
		solc PROPERTIES
		LINK_FLAGS "-static-libgcc -static-libstdc++"
	)
endif()
