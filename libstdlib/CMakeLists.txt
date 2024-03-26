# This will re-generate the headers if any file within src was modified.
set_directory_properties(PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(STDLIB stub)
set(GENERATED_STDLIB_HEADERS)
foreach(src IN LISTS STDLIB)
    set(STDLIB_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/${src}.sol)
    file(READ ${STDLIB_FILE} STDLIB_FILE_CONTENT HEX)
    string(REGEX MATCHALL ".." STDLIB_FILE_CONTENT "${STDLIB_FILE_CONTENT}")
    list(REMOVE_ITEM STDLIB_FILE_CONTENT "0d")
    string(REGEX REPLACE ";" ",\n\t0x" STDLIB_FILE_CONTENT "${STDLIB_FILE_CONTENT}")
    set(STDLIB_FILE_CONTENT "0x${STDLIB_FILE_CONTENT}")
    set(STDLIB_FILE_NAME ${src})
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/stdlib.src.h.in" ${PROJECT_BINARY_DIR}/include/libstdlib/${src}.h NEWLINE_STYLE LF @ONLY)
    list(APPEND GENERATED_STDLIB_HEADERS ${PROJECT_BINARY_DIR}/include/libstdlib/${src}.h)
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/stdlib.h.in" ${PROJECT_BINARY_DIR}/include/libstdlib/stdlib.h NEWLINE_STYLE LF @ONLY)
