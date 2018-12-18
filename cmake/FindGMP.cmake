find_library(GMP_LIBRARY NAMES gmp)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_LIBRARY)

if(GMP_FOUND AND NOT TARGET GMP::GMP)
    add_library(GMP::GMP UNKNOWN IMPORTED)
    set_property(TARGET GMP::GMP PROPERTY IMPORTED_LOCATION ${GMP_LIBRARY})
endif()
