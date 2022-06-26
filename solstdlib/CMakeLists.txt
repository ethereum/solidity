# This will re-generate the headers if any file within src was modified.
set_directory_properties(PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/solstdlib/src/)

set(STDLIB precompiles)
set(GENERATED_STDLIB_HEADERS)
foreach(src IN LISTS STDLIB)
	set(STDLIB_FILE ${CMAKE_SOURCE_DIR}/solstdlib/src/${src}.sol)
	file(READ ${STDLIB_FILE} STDLIB_FILE_CONTENT HEX)
	string(REGEX MATCHALL ".." STDLIB_FILE_CONTENT "${STDLIB_FILE_CONTENT}")
	string(REGEX REPLACE ";" ",\n\t0x" STDLIB_FILE_CONTENT "${STDLIB_FILE_CONTENT}")
	set(STDLIB_FILE_CONTENT "0x${STDLIB_FILE_CONTENT}")
	set(STDLIB_FILE_NAME ${src})
	configure_file("${CMAKE_SOURCE_DIR}/solstdlib/solstdlib.src.h.in" ${CMAKE_BINARY_DIR}/include/solstdlib/${src}.h @ONLY)
	list(APPEND GENERATED_STDLIB_HEADERS ${CMAKE_BINARY_DIR}/include/solstdlib/${src}.h)
endforeach()

configure_file("${CMAKE_SOURCE_DIR}/solstdlib/solstdlib.h.in" ${CMAKE_BINARY_DIR}/include/solstdlib.h @ONLY)

#add_library(stdlib
#	${GENERATED_STDLIB_HEADERS}
#)
