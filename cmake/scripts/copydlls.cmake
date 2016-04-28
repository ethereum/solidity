# this module expects
# DLLS
# CONF
# DESTINATION

# example usage:
# cmake -DDLL_DEBUG=xd.dll -DDLL_RELEASE=x.dll -DCONFIGURATION=Release -DDESTINATION=dest -P scripts/copydlls.cmake

# this script is created cause we do not know configuration in multiconfiguration generators at cmake configure phase ;)

if ("${CONF}" STREQUAL "Debug")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${DLL_DEBUG}" "${DESTINATION}")
	
	# hack, copy it twice. with and without d.dll suffix
	# at first let's get the file name part
	get_filename_component(DLL_DEBUG_D_NAME ${DLL_DEBUG} NAME)
	string(REPLACE "d.dll" ".dll" DLL_DEBUG_D_NAME ${DLL_DEBUG_D_NAME})
	string(REPLACE "_.dll" ".dll" DLL_DEBUG_D_NAME ${DLL_DEBUG_D_NAME})
	
	set(DESTINATION_D "${DESTINATION}/${DLL_DEBUG_D_NAME}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${DLL_DEBUG}" "${DESTINATION_D}")
else ()
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${DLL_RELEASE}" "${DESTINATION}")
endif()
