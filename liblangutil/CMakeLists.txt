# Solidity Commons Library (Solidity related sharing bits between libsolidity and libyul)
file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(langutil ${sources} ${headers})
target_link_libraries(langutil PUBLIC devcore)
