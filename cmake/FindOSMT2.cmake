if (USE_OSMT2)
	find_path(OSMT2_INCLUDE_DIR opensmt/opensmt2.h)
	find_library(OSMT2_LIBRARY NAMES opensmt)
    include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(OSMT2 DEFAULT_MSG OSMT2_LIBRARY OSMT2_INCLUDE_DIR)
	if(OSMT2_FOUND)
		# OSMT2 depends on GMP.
		# We can assume that GMP is present on the system,
        # so we quietly try to find it and link against it, if it is present.
        find_package(GMP QUIET)

		set(OSMT2_LIBRARIES ${OSMT2_LIBRARY})

        if (GMP_FOUND)
			set(OSMT2_LIBRARIES ${OSMT2_LIBRARIES} GMP::GMP)
        endif ()

		if (NOT TARGET OSMT2::OSMT2)
			add_library(OSMT2::OSMT2 UNKNOWN IMPORTED)
			set_property(TARGET OSMT2::OSMT2 PROPERTY IMPORTED_LOCATION ${OSMT2_LIBRARY})
			set_property(TARGET OSMT2::OSMT2 PROPERTY INTERFACE_LINK_LIBRARIES ${OSMT2_LIBRARIES})
			set_property(TARGET OSMT2::OSMT2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OSMT2_INCLUDE_DIR})
        endif()
    endif()
else()
	set(OSMT2_FOUND FALSE)
endif()
