add_executable(lllc main.cpp)
target_link_libraries(lllc PRIVATE lll ${Boost_SYSTEM_LIBRARY})

if (INSTALL_LLLC)
	include(GNUInstallDirs)
	install(TARGETS lllc DESTINATION ${CMAKE_INSTALL_BINDIR})

	if(LLLC_LINK_STATIC AND UNIX AND NOT APPLE)
		# Produce lllc as statically linked binary (includes C/C++ standard libraries)
		# This is not supported on macOS, see
		# https://developer.apple.com/library/content/qa/qa1118/_index.html.
		set_target_properties(
			lllc PROPERTIES
			LINK_FLAGS -static
			LINK_SEARCH_START_STATIC ON
			LINK_SEARCH_END_STATIC ON
		)
	endif()
endif()
