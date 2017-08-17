file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(lll ${sources} ${headers})
target_link_libraries(lll PUBLIC evmasm devcore)
target_include_directories(lll PUBLIC ..)
