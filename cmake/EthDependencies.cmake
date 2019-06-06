if (DEFINED MSVC)
	# by defining CMAKE_PREFIX_PATH variable, cmake will look for dependencies first in our own repository before looking in system paths like /usr/local/ ...
	# this must be set to point to the same directory as $ETH_DEPENDENCY_INSTALL_DIR in /extdep directory

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0)
		set (ETH_DEPENDENCY_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../extdep/install/windows/x64")
	else()
		get_filename_component(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/../deps/install" ABSOLUTE)
		set(ETH_DEPENDENCY_INSTALL_DIR
			"${DEPS_DIR}/x64"					# Old location for deps.
			"${DEPS_DIR}/win64"					# New location for deps.
			"${DEPS_DIR}/win64/Release/share"	# LLVM shared cmake files.
		)
	endif()
	set (CMAKE_PREFIX_PATH ${ETH_DEPENDENCY_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
endif()

# custom cmake scripts
set(ETH_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(ETH_SCRIPTS_DIR ${ETH_CMAKE_DIR}/scripts)

## use multithreaded boost libraries, with -mt suffix
set(Boost_USE_MULTITHREADED ON)
option(Boost_USE_STATIC_LIBS "Link Boost statically" ON)

set(BOOST_COMPONENTS "regex;filesystem;unit_test_framework;program_options;system")

find_package(Boost 1.65.0 QUIET REQUIRED COMPONENTS ${BOOST_COMPONENTS})

# make sure we actually get all required imported targets for boost
if (NOT TARGET Boost::boost) # header only target
	add_library(Boost::boost INTERFACE IMPORTED)
	target_include_directories(Boost::boost INTERFACE ${Boost_INCLUDE_DIRS})
endif()
foreach (BOOST_COMPONENT IN LISTS BOOST_COMPONENTS)
	if (NOT TARGET Boost::${BOOST_COMPONENT})
		add_library(Boost::${BOOST_COMPONENT} UNKNOWN IMPORTED)
		string(TOUPPER ${BOOST_COMPONENT} BOOST_COMPONENT_UPPER)
		set_property(TARGET Boost::${BOOST_COMPONENT} PROPERTY IMPORTED_LOCATION ${Boost_${BOOST_COMPONENT_UPPER}_LIBRARY})
		set_property(TARGET Boost::${BOOST_COMPONENT} PROPERTY INTERFACE_LINK_LIBRARIES ${Boost_${BOOST_COMPONENT_UPPER}_LIBRARIES})
		set_property(TARGET Boost::${BOOST_COMPONENT} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    endif()
endforeach()
