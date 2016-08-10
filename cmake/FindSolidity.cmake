# Find Solidity
#
# Find the solidity includes and library
#
# This module defines
#  Solidity_XXX_LIBRARIES, the libraries needed to use solidity.
#  SOLIDITY_INCLUDE_DIRS

include(EthUtils)
set(LIBS solidity;lll;solevmasm)

set(Solidity_INCLUDE_DIRS "${SOL_DIR}")

# if the project is a subset of main cpp-ethereum project
# use same pattern for variables as Boost uses
if ((DEFINED solidity_VERSION) OR (DEFINED cpp-ethereum_VERSION))

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)
		set ("Solidity_${L}_LIBRARIES" ${l})
	endforeach()

else()

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)
		find_library(Solidity_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
			PATH_SUFFIXES "lib${l}" "${l}" "lib${l}/Debug" "lib${l}/Release"
			NO_DEFAULT_PATH
		)

		set(Solidity_${L}_LIBRARIES ${Solidity_${L}_LIBRARY})

		if (DEFINED MSVC)
			find_library(Solidity_${L}_LIBRARY_DEBUG
				NAMES ${l}
				PATHS ${CMAKE_LIBRARY_PATH}
				PATH_SUFFIXES "lib${l}/Debug" 
				NO_DEFAULT_PATH
			)
			eth_check_library_link(Solidity_${L})
		endif()
	endforeach()

endif()
