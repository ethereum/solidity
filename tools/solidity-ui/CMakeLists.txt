include_directories(${CMAKE_SOURCE_DIR}/tools/solidity-ui)

include(solidity-ui/imgui)
include(solidity-ui/zep)
include(solidity-ui/imgui_memory_editor)
include(solidity-ui/implot)

set(SOLIDITY_UI_ASSETS JetBrainsMono-Regular.ttf)
set(GENERATED_SOLIDITY_UI_ASSETS_HEADERS)
foreach(asset IN LISTS SOLIDITY_UI_ASSETS)
    set(SOLIDITY_UI_ASSET_FILE ${CMAKE_SOURCE_DIR}/tools/solidity-ui/assets/${asset})
    file(READ ${SOLIDITY_UI_ASSET_FILE} SOLIDITY_UI_ASSET_FILE_CONTENT HEX)
    string(REGEX MATCHALL ".." SOLIDITY_UI_ASSET_FILE_CONTENT "${SOLIDITY_UI_ASSET_FILE_CONTENT}")
    string(REGEX REPLACE ";" ",\n\t0x" SOLIDITY_UI_ASSET_FILE_CONTENT "${SOLIDITY_UI_ASSET_FILE_CONTENT}")
    set(SOLIDITY_UI_ASSET_FILE_CONTENT "0x${SOLIDITY_UI_ASSET_FILE_CONTENT}")
    set(SOLIDITY_UI_ASSET_FILE_NAME ${asset})
    string(REGEX REPLACE "\\." "_" SOLIDITY_UI_ASSET_FILE_NAME "${SOLIDITY_UI_ASSET_FILE_NAME}")
    string(REGEX REPLACE "-" "_" SOLIDITY_UI_ASSET_FILE_NAME "${SOLIDITY_UI_ASSET_FILE_NAME}")
    configure_file("${CMAKE_SOURCE_DIR}/cmake/solidity-ui/asset.in" ${CMAKE_BINARY_DIR}/tools/solidity-ui/assets/${asset}.h @ONLY)
    list(APPEND SOLIDITY_UI_ASSETS_HEADERS ${CMAKE_BINARY_DIR}/tools/solidity-ui/assets/${asset}.h)
endforeach()

set(sources
        ${SOLIDITY_UI_ASSETS_HEADERS}
        main.cpp
        )
add_executable(solidity-ui ${sources})
target_link_libraries(solidity-ui PRIVATE solcli SYSTEM imgui zep imgui_memory_editor implot)
target_compile_definitions(solidity-ui PUBLIC CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_include_directories(solidity-ui PUBLIC ${CMAKE_BINARY_DIR}/tools/solidity-ui)


include(GNUInstallDirs)
install(TARGETS solidity-ui DESTINATION "${CMAKE_INSTALL_BINDIR}")

if (SOLC_LINK_STATIC AND UNIX AND NOT APPLE)
    # Produce solc as statically linked binary (includes C/C++ standard libraries)
    # This is not supported on macOS, see
    # https://developer.apple.com/library/content/qa/qa1118/_index.html.
    set_target_properties(
            solidity-ui PROPERTIES
            LINK_FLAGS -static
            LINK_SEARCH_START_STATIC ON
            LINK_SEARCH_END_STATIC ON
    )
elseif (SOLC_STATIC_STDLIBS AND UNIX AND NOT APPLE)
    set_target_properties(
            solidity-ui PROPERTIES
            LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
endif ()
