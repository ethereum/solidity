find_library(CLN_LIBRARY NAMES cln)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CLN DEFAULT_MSG CLN_LIBRARY)

if(CLN_FOUND AND NOT TARGET CLN::CLN)
    add_library(CLN::CLN UNKNOWN IMPORTED)
    set_property(TARGET CLN::CLN PROPERTY IMPORTED_LOCATION ${CLN_LIBRARY})
endif()
