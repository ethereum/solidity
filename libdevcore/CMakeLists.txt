file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

add_library(devcore ${sources} ${headers})
target_link_libraries(devcore PRIVATE ${Boost_FILESYSTEM_LIBRARIES} ${Boost_REGEX_LIBRARIES} ${Boost_SYSTEM_LIBRARIES} Threads::Threads)
target_include_directories(devcore SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(devcore PRIVATE ..)
add_dependencies(devcore solidity_BuildInfo.h)
