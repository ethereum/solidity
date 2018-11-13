if (USE_Z3)
    find_path(Z3_INCLUDE_DIR NAMES z3++.h PATH_SUFFIXES z3)
    find_library(Z3_LIBRARY NAMES z3 )
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Z3 DEFAULT_MSG Z3_LIBRARY Z3_INCLUDE_DIR)
else()
    set(Z3_FOUND FALSE)
endif()
# TODO: Create IMPORTED library for Z3.
