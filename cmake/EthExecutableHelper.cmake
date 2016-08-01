#
# this function requires the following variables to be specified:
# ETH_VERSION
# PROJECT_NAME
# PROJECT_VERSION
# PROJECT_COPYRIGHT_YEAR
# PROJECT_VENDOR
# PROJECT_DOMAIN_SECOND
# PROJECT_DOMAIN_FIRST
# SRC_LIST
# HEADERS
#
# params:
# ICON
#

macro(eth_add_executable EXECUTABLE)
	set (extra_macro_args ${ARGN})
	set (options)
	set (one_value_args ICON)
	set (multi_value_args UI_RESOURCES WIN_RESOURCES)
	cmake_parse_arguments (ETH_ADD_EXECUTABLE "${options}" "${one_value_args}" "${multi_value_args}" "${extra_macro_args}")

	if (APPLE)

		add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SRC_LIST} ${HEADERS} ${ETH_ADD_EXECUTABLE_UI_RESOURCES})
		set(PROJECT_VERSION "${ETH_VERSION}")
		set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_COPYRIGHT "${PROJECT_COPYRIGHT_YEAR} ${PROJECT_VENDOR}")
		set(MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_DOMAIN_SECOND}.${PROJECT_DOMAIN_FIRST}")
		set(MACOSX_BUNDLE_BUNDLE_NAME ${EXECUTABLE})
		set(MACOSX_BUNDLE_ICON_FILE ${ETH_ADD_EXECUTABLE_ICON})
		set_target_properties(${EXECUTABLE} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/EthereumMacOSXBundleInfo.plist.in")
		set_source_files_properties(${EXECUTABLE} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS)
		set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	else ()
		add_executable(${EXECUTABLE} ${ETH_ADD_EXECUTABLE_UI_RESOURCES}  ${ETH_ADD_EXECUTABLE_WIN_RESOURCES} ${SRC_LIST} ${HEADERS})
	endif()

endmacro()

macro(eth_simple_add_executable EXECUTABLE)
	add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

	# Apple does not support statically linked binaries on OS X.   That means
	# that we can only statically link against our external libraries, but
	# we cannot statically link against the C++ runtime libraries and other
	# platform libraries (as is possible on Windows and Alpine Linux) to produce
	# an entirely transportable binary.
	#
	# See https://developer.apple.com/library/mac/qa/qa1118/_index.html for more info.
	#
	# GLIBC also appears not to support static linkage too, which probably means that
	# Debian and Ubuntu will only be able to do partially-statically linked
	# executables too, just like OS X.
	#
	# For OS X, at the time of writing, we are left with the following dynamically
	# linked dependencies, of which curl and libz might still be fixable:
	#
	# /usr/lib/libc++.1.dylib
	# /usr/lib/libSystem.B.dylib
	# /usr/lib/libcurl.4.dylib
	# /usr/lib/libz.1.dylib
	#
	if (STATIC_LINKING AND NOT APPLE)
		set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")
		set_target_properties(${EXECUTABLE} PROPERTIES LINK_SEARCH_START_STATIC 1)
		set_target_properties(${EXECUTABLE} PROPERTIES LINK_SEARCH_END_STATIC 1)
	endif()
endmacro()

macro(eth_copy_dll EXECUTABLE DLL)
	# dlls must be unsubstitud list variable (without ${}) in format
	# optimized;path_to_dll.dll;debug;path_to_dlld.dll
	if(DEFINED MSVC)
		list(GET ${DLL} 1 DLL_RELEASE)
		list(GET ${DLL} 3 DLL_DEBUG)
		add_custom_command(TARGET ${EXECUTABLE}
			PRE_BUILD
			COMMAND ${CMAKE_COMMAND} ARGS
			-DDLL_RELEASE="${DLL_RELEASE}"
			-DDLL_DEBUG="${DLL_DEBUG}"
			-DCONF="$<CONFIGURATION>"
			-DDESTINATION="${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
			-P "${ETH_SCRIPTS_DIR}/copydlls.cmake"
		)
	endif()
endmacro()

macro(eth_copy_dlls EXECUTABLE)
	foreach(dll ${ARGN})
		eth_copy_dll(${EXECUTABLE} ${dll})
	endforeach(dll)
endmacro()


macro(eth_install_executable EXECUTABLE)

	if (APPLE)

		# TODO - Why is this different than the branch Linux below, which has the RUNTIME keyword too?
		install(TARGETS ${EXECUTABLE} DESTINATION bin)

	elseif (DEFINED MSVC)

		set(COMPONENT ${EXECUTABLE})

		install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debug/"
			DESTINATION .
			CONFIGURATIONS Debug
			COMPONENT ${COMPONENT}
		)

		install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/"
			DESTINATION .
			CONFIGURATIONS Release
			COMPONENT ${COMPONENT}
		)

		install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/"
			DESTINATION .
			CONFIGURATIONS RelWithDebInfo
			COMPONENT ${COMPONENT}
		)

	else()
		install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin)
	endif ()

endmacro()

macro (eth_name KEY VALUE)
	if (NOT (APPLE OR WIN32))
		string(TOLOWER ${VALUE} LVALUE )
		set(${KEY} ${LVALUE})
	else()
		set(${KEY} ${VALUE})
	endif()
endmacro()
