file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE toolsources "tools/*.cpp")
list(REMOVE_ITEM sources ${toolsources})
file(GLOB_RECURSE headers "*.h")

add_executable(soltest ${sources} ${headers})
target_link_libraries(soltest PRIVATE libsolc solidity lll evmasm devcore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_executable(solfuzzer tools/fuzzer.cpp)
target_link_libraries(solfuzzer PRIVATE libsolc evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})

add_executable(syntaxTestTool tools/syntaxTestTool.cpp TestHelper.cpp libsolidity/SyntaxTest.cpp libsolidity/AnalysisFramework.cpp)
target_link_libraries(syntaxTestTool PRIVATE libsolc solidity evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
