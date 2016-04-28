# Find Utils
#
# Find the utils includes and library
#
# This module defines
#  Utils_XXX_LIBRARIES, the libraries needed to use ethereum.
#  Utils_INCLUDE_DIRS

include(EthUtils)
set(LIBS secp256k1;scrypt)

set(Utils_INCLUDE_DIRS "${UTILS_DIR}")

# if the project is a subset of main cpp-ethereum project
# use same pattern for variables as Boost uses
if (DEFINED cpp-ethereum_VERSION)

	foreach (l ${LIBS}) 
		string(TOUPPER ${l} L)
		set ("Utils_${L}_LIBRARIES" ${l})
	endforeach()

else()

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)

		find_library(Utils_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
			PATH_SUFFIXES "lib${l}" "${l}" "lib${l}/Debug" "${l}/Debug" "lib${l}/Release" "${l}/Release" 
			NO_DEFAULT_PATH
		)

		set(Utils_${L}_LIBRARIES ${Utils_${L}_LIBRARY})

		if (DEFINED MSVC)
			find_library(Utils_${L}_LIBRARY_DEBUG
				NAMES ${l}
				PATHS ${CMAKE_LIBRARY_PATH}
				PATH_SUFFIXES "lib${l}/Debug" "${l}/Debug" 
				NO_DEFAULT_PATH
			)
			eth_check_library_link(Utils_${L})
		endif()
	endforeach()

endif()

