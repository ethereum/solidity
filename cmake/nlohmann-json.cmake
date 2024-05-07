include(ExternalProject)

ExternalProject_Add(nlohmann-json-project
        DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/nlohmann/nlohmann"
        DOWNLOAD_NAME json.hpp
        DOWNLOAD_NO_EXTRACT 1
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
        URL_HASH SHA256=9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6
        CMAKE_COMMAND true
        BUILD_COMMAND true
        INSTALL_COMMAND true
)

# Create nlohmann-json imported library
add_library(nlohmann-json INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/nlohmann)  # Must exist.
set_target_properties(nlohmann-json PROPERTIES
        INTERFACE_COMPILE_OPTIONS "\$<\$<CXX_COMPILER_ID:MSVC>:/permissive->"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES  ${CMAKE_SOURCE_DIR}/deps/nlohmann
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/nlohmann)
add_dependencies(nlohmann-json nlohmann-json-project)