file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

file(GLOB contracts_sources "contracts/*.cpp")
file(GLOB contracts_headers "contracts/*.h")
file(GLOB libdevcore_sources "libdevcore/*.cpp")
file(GLOB libdevcore_headers "libdevcore/*.h")
file(GLOB liblangutil_sources "liblangutil/*.cpp")
file(GLOB liblangutil_headers "liblangutil/*.h")
file(GLOB libevmasm_sources "libevmasm/*.cpp")
file(GLOB libevmasm_headers "libevmasm/*.h")
file(GLOB libyul_sources "libyul/*.cpp")
file(GLOB libyul_headers "libyul/*.h")
if (LLL)
    file(GLOB liblll_sources "liblll/*.cpp")
    file(GLOB liblll_headers "liblll/*.h")
endif()
file(GLOB libsolidity_sources "libsolidity/*.cpp")
file(GLOB libsolidity_headers "libsolidity/*.h")
file(GLOB libsolidity_util_sources "libsolidity/util/*.cpp")
file(GLOB libsolidity_util_headers "libsolidity/util/*.h")

add_executable(soltest ${sources} ${headers}
    ${contracts_sources} ${contracts_headers}
    ${libdevcore_sources} ${libdevcore_headers}
    ${liblangutil_sources} ${liblangutil_headers}
    ${libevmasm_sources} ${libevmasm_headers}
    ${libyul_sources} ${libyul_headers}
    ${liblll_sources} ${liblll_headers}
    ${libsolidity_sources} ${libsolidity_headers}
    ${libsolidity_util_sources} ${libsolidity_util_headers}
)
target_link_libraries(soltest PRIVATE libsolc yul solidity evmasm devcore ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

if (LLL)
    target_link_libraries(soltest PRIVATE lll)
    target_compile_definitions(soltest PRIVATE HAVE_LLL=1)
endif()

if (NOT Boost_USE_STATIC_LIBS)
    target_compile_definitions(soltest PUBLIC -DBOOST_TEST_DYN_LINK)
endif()

add_subdirectory(tools)
