include(ExternalProject)

ExternalProject_Add(binaryen
    PREFIX deps
    DOWNLOAD_NAME binaryen-1.37.10.tar.gz
    DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/deps/downloads
    URL https://github.com/WebAssembly/binaryen/archive/1.37.10.tar.gz
    URL_HASH SHA256=5a4e622166309d6df4fca0916aaa662c886ddc7bce34e6cc21f11d87ea543c7a
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DBUILD_STATIC_LIB=ON
    # Overwtire build and install commands to force Release build on MSVC.
    BUILD_COMMAND cmake --build <BINARY_DIR> --config Release
    INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target install
)

# Create jsoncpp imported library
ExternalProject_Get_Property(binaryen INSTALL_DIR)
ExternalProject_Get_Property(binaryen SOURCE_DIR)
add_library(Binaryen::Binaryen STATIC IMPORTED)
set(BINARYEN_LIBRARY ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}binaryen${CMAKE_STATIC_LIBRARY_SUFFIX})
# Use source dir because binaryen only installs single header with C API.
set(BINARYEN_INCLUDE_DIR ${SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${BINARYEN_INCLUDE_DIR})  # Must exist.
set_property(TARGET Binaryen::Binaryen PROPERTY IMPORTED_LOCATION ${BINARYEN_LIBRARY})
set_property(TARGET Binaryen::Binaryen PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BINARYEN_INCLUDE_DIR})
add_dependencies(Binaryen::Binaryen binaryen)
unset(INSTALL_DIR)
