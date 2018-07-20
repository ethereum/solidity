include(ExternalProject)

ExternalProject_Add(nlohmann-json
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/nlohmann/json"
    DOWNLOAD_NAME json.hpp
    DOWNLOAD_NO_EXTRACT 1
    URL https://github.com/nlohmann/json/releases/download/v3.10.2/json.hpp
    URL_HASH SHA256=059743e48b37e41579ee3a92e82e984bfa0d2a9a2b20b175d04db8089f46f047
    CMAKE_COMMAND true
    BUILD_COMMAND true
    INSTALL_COMMAND true
)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/deps/nlohmann)
