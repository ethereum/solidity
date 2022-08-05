FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG fc0fd112467c2be84bc56daa57612b0c579ff1af
)
FetchContent_GetProperties(implot)
if(NOT implot_POPULATED)
    FetchContent_Populate(implot)
endif()

add_library(implot)
target_sources(implot PRIVATE
        ${implot_SOURCE_DIR}/implot.h
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_internal.h
        ${implot_SOURCE_DIR}/implot_items.cpp)
target_compile_options(implot PRIVATE "-Wno-sign-conversion")
target_compile_options(implot PRIVATE "-Wno-extra-semi")
target_compile_options(implot PRIVATE "-Wno-implicit-fallthrough")
target_compile_options(implot PRIVATE "-Wno-null-arithmetic")
target_link_libraries(implot PRIVATE imgui)
target_include_directories(implot INTERFACE ${implot_SOURCE_DIR})
add_dependencies(implot imgui)