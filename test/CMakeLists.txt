file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

file(GLOB contracts_sources "contracts/*.cpp")
file(GLOB contracts_headers "contracts/*.h")
file(GLOB libdevcore_sources "libdevcore/*.cpp")
file(GLOB libdevcore_headers "libdevcore/*.h")
file(GLOB libevmasm_sources "libevmasm/*.cpp")
file(GLOB libevmasm_headers "libevmasm/*.h")
file(GLOB libjulia_sources "libjulia/*.cpp")
file(GLOB libjulia_headers "libjulia/*.h")
file(GLOB liblll_sources "liblll/*.cpp")
file(GLOB liblll_headers "liblll/*.h")
file(GLOB libsolidity_sources "libsolidity/*.cpp")
file(GLOB libsolidity_headers "libsolidity/*.h")

add_executable(soltest ${sources} ${headers}
    ${contracts_sources} ${contracts_headers}
    ${libdevcore_sources} ${libdevcore_headers}
    ${libevmasm_sources} ${libevmasm_headers}
    ${libjulia_sources} ${libjulia_headers}
    ${liblll_sources} ${liblll_headers}
    ${libsolidity_sources} ${libsolidity_headers}
)
target_link_libraries(soltest PRIVATE libsolc solidity lll evmasm devcore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

if (NOT Boost_USE_STATIC_LIBS)
    target_compile_definitions(soltest PUBLIC -DBOOST_TEST_DYN_LINK)
endif()

add_subdirectory(tools)
