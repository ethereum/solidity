set(
	sources
	CommandLineInterface.cpp CommandLineInterface.h
	main.cpp
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	# Since we do not want to depend on networking code on production binaries,
	# this feature is only available when creating a debug build.
	# the LSP's TCP listener is exclusively used more convenient debugging.
	option(SOLC_LSP_TCP "Solidity LSP: Enables TCP listener support (should only be eanbled for debugging purposes)." OFF)
	if(SOLC_LSP_TCP)
		set(SOLC_DEFINITIONS SOLC_LSP_TCP=1)
		list(APPEND sources LSPTCPTransport.cpp LSPTCPTransport.h)
		set(SOLC_LSP_LIBS pthread)
	endif()
endif()

add_executable(solc ${sources})
target_link_libraries(solc PRIVATE solidity Boost::boost Boost::program_options ${SOLC_LSP_LIBS})
target_compile_definitions(solc PRIVATE ${SOLC_DEFINITIONS})

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
