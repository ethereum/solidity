set(BUILD_EXTENSIONS OFF)
set(BUILD_DEMOS OFF)
add_compile_definitions(ZEP_SINGLE_HEADER_BUILD=1 ZEP_FEATURE_CPP_FILE_SYSTEM=1)
FetchContent_Declare(
        zep
        GIT_REPOSITORY https://github.com/Rezonality/zep.git
        GIT_TAG 46a783d6bc14f2dad2660c98ffcfddec5500b085
)
FetchContent_GetProperties(zep)
if(NOT zep_POPULATED)
    FetchContent_Populate(zep)
endif()

add_library(zep INTERFACE IMPORTED)
target_include_directories(zep INTERFACE ${zep_SOURCE_DIR}/include)
