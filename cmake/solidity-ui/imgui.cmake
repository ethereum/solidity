FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG 58eb40db76783f5da09e592ca3eb421f4f2197e3
)
FetchContent_MakeAvailable(imgui)
add_library(imgui
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imstb_textedit.h
        ${imgui_SOURCE_DIR}/imstb_truetype.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
        ${CMAKE_SOURCE_DIR}/tools/solidity-ui/platform/macos/imgui_impl_sdl.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
target_link_libraries(imgui PRIVATE SDL2::SDL2 OpenGL::GL)
#target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})
