# Until we have a clear separation, libjulia has to be included here
file(GLOB_RECURSE sources "*.cpp" "../libjulia/*.cpp")
file(GLOB_RECURSE headers "*.h" "../libjulia/*.h")

add_library(solidity ${sources} ${headers})
target_link_libraries(solidity PUBLIC evmasm devcore)
target_include_directories(solidity PUBLIC ..)
