file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(evmasm ${sources} ${headers})
target_link_libraries(evmasm PUBLIC devcore)
