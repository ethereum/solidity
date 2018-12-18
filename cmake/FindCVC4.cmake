if (USE_CVC4)
    find_path(CVC4_INCLUDE_DIR cvc4/cvc4.h)
    find_library(CVC4_LIBRARY NAMES cvc4)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(CVC4 DEFAULT_MSG CVC4_LIBRARY CVC4_INCLUDE_DIR)
    if(CVC4_FOUND)
        # CVC4 may depend on either CLN or GMP.
        # We can assume that the one it requires is present on the system,
        # so we quietly try to find both and link against them, if they are
        # present.
        find_package(CLN QUIET)
        find_package(GMP QUIET)

        set(CVC4_LIBRARIES ${CVC4_LIBRARY})

        if (CLN_FOUND)
            set(CVC4_LIBRARIES ${CVC4_LIBRARIES} CLN::CLN)
        endif ()

        if (GMP_FOUND)
            set(CVC4_LIBRARIES ${CVC4_LIBRARIES} GMP::GMP)
        endif ()

        if (NOT TARGET CVC4::CVC4)
            add_library(CVC4::CVC4 UNKNOWN IMPORTED)
            set_property(TARGET CVC4::CVC4 PROPERTY IMPORTED_LOCATION ${CVC4_LIBRARY})
            set_property(TARGET CVC4::CVC4 PROPERTY INTERFACE_LINK_LIBRARIES ${CVC4_LIBRARIES})
            set_property(TARGET CVC4::CVC4 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CVC4_INCLUDE_DIR})
        endif()
    endif()
else()
    set(CVC4_FOUND FALSE)
endif()
