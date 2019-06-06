if (USE_Z3)
    # Save and clear Z3_FIND_VERSION, since the
    # Z3 config module cannot handle version requirements.
    set(Z3_FIND_VERSION_ORIG ${Z3_FIND_VERSION})
    set(Z3_FIND_VERSION)
    # Try to find Z3 using its stock cmake files.
    find_package(Z3 QUIET CONFIG)
    # Restore Z3_FIND_VERSION for find_package_handle_standard_args.
    set(Z3_FIND_VERSION ${Z3_FIND_VERSION_ORIG})
    set(Z3_FIND_VERSION_ORIG)

    include(FindPackageHandleStandardArgs)

    if (Z3_FOUND)
        set(Z3_VERSION ${Z3_VERSION_STRING})
        find_package_handle_standard_args(Z3 CONFIG_MODE)
    else()
        find_path(Z3_INCLUDE_DIR NAMES z3++.h PATH_SUFFIXES z3)
        find_library(Z3_LIBRARY NAMES z3)
        find_program(Z3_EXECUTABLE z3 PATH_SUFFIXES bin)

        if(Z3_INCLUDE_DIR AND Z3_LIBRARY AND Z3_EXECUTABLE)
            execute_process (COMMAND ${Z3_EXECUTABLE} -version
                OUTPUT_VARIABLE libz3_version_str
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            string(REGEX REPLACE "^Z3 version ([0-9.]+).*" "\\1"
                   Z3_VERSION_STRING "${libz3_version_str}")
            unset(libz3_version_str)
        endif()
        mark_as_advanced(Z3_VERSION_STRING z3_DIR)

        find_package_handle_standard_args(Z3
            REQUIRED_VARS Z3_LIBRARY Z3_INCLUDE_DIR
            VERSION_VAR Z3_VERSION_STRING)

        if (NOT TARGET z3::libz3)
            add_library(z3::libz3 UNKNOWN IMPORTED)
            set_property(TARGET z3::libz3 PROPERTY IMPORTED_LOCATION ${Z3_LIBRARY})
            set_property(TARGET z3::libz3 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Z3_INCLUDE_DIR})
        endif()
    endif()
else()
    set(Z3_FOUND FALSE)
endif()
