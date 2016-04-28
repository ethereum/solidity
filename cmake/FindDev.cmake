# Find Dev
#
# Find the dev includes and library
#
# This module defines
#  Dev_XXX_LIBRARIES, the libraries needed to use ethereum.
#  Dev_INCLUDE_DIRS

include(EthUtils)
set(LIBS devcore;devcrypto;p2p)

set(Dev_INCLUDE_DIRS "${DEV_DIR}")

# if the project is a subset of main cpp-ethereum project
# use same pattern for variables as Boost uses
if ((DEFINED cpp-ethereum_VERSION) OR (DEFINED dev_VERSION))

	foreach (l ${LIBS}) 
		string(TOUPPER ${l} L)
		set ("Dev_${L}_LIBRARIES" ${l})
	endforeach()

else()

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)

		find_library(Dev_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
			PATH_SUFFIXES "lib${l}" "${l}" "lib${l}/Debug" "lib${l}/Release" 
			NO_DEFAULT_PATH
		)

		set(Dev_${L}_LIBRARIES ${Dev_${L}_LIBRARY})

		if (DEFINED MSVC)
			find_library(Dev_${L}_LIBRARY_DEBUG
				NAMES ${l}
				PATHS ${CMAKE_LIBRARY_PATH}
				PATH_SUFFIXES "lib${l}/Debug" 
				NO_DEFAULT_PATH
			)
			eth_check_library_link(Dev_${L})
		endif()
	endforeach()

endif()

