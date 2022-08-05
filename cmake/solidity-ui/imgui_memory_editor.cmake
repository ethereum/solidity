FetchContent_Declare(
        imgui_memory_editor
        GIT_REPOSITORY https://github.com/ocornut/imgui_club.git
        GIT_TAG d4cd9896e15a03e92702a578586c3f91bbde01e8
)
FetchContent_GetProperties(imgui_memory_editor)
if(NOT imgui_memory_editor_POPULATED)
    FetchContent_Populate(imgui_memory_editor)
endif()

add_library(imgui_memory_editor INTERFACE IMPORTED)
target_include_directories(imgui_memory_editor INTERFACE ${imgui_memory_editor_SOURCE_DIR}/imgui_memory_editor)
