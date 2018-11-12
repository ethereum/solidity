file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")

add_library(yul ${sources} ${headers})
target_link_libraries(yul PUBLIC devcore evmasm solcommon)
