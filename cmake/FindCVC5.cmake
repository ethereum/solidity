if (USE_CVC5)
    find_path(CVC5_INCLUDE_DIR cvc5/cvc5.h)
    find_library(CVC5_LIBRARY NAMES cvc5)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(CVC5 DEFAULT_MSG CVC5_LIBRARY CVC5_INCLUDE_DIR)
    if(CVC5_FOUND)
        # CVC5 may depend on either CLN or GMP.
        # We can assume that the one it requires is present on the system,
        # so we quietly try to find both and link against them, if they are
        # present.
        find_package(CLN QUIET)
        find_package(GMP QUIET)

        set(CVC5_LIBRARIES ${CVC5_LIBRARY})

        if (CLN_FOUND)
            set(CVC5_LIBRARIES ${CVC5_LIBRARIES} CLN::CLN)
        endif ()

        if (GMP_FOUND)
            set(CVC5_LIBRARIES ${CVC5_LIBRARIES} GMP::GMP)
        endif ()

        if (NOT TARGET CVC5::CVC5)
            add_library(CVC5::CVC5 UNKNOWN IMPORTED)
            set_property(TARGET CVC5::CVC5 PROPERTY IMPORTED_LOCATION ${CVC5_LIBRARY})
            set_property(TARGET CVC5::CVC5 PROPERTY INTERFACE_LINK_LIBRARIES ${CVC5_LIBRARIES})
            set_property(TARGET CVC5::CVC5 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CVC5_INCLUDE_DIR})
        endif()
    endif()
else()
    set(CVC5_FOUND FALSE)
endif()
