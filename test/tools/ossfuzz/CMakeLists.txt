add_custom_target(ossfuzz)
add_dependencies(ossfuzz
        solc_opt_ossfuzz
        solc_noopt_ossfuzz
        const_opt_ossfuzz
        strictasm_diff_ossfuzz
        )

#[[FuzzingEngine.a is provided by oss-fuzz's Dockerized build environment]]
add_executable(solc_opt_ossfuzz solc_opt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(solc_opt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

add_executable(solc_noopt_ossfuzz solc_noopt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(solc_noopt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

add_executable(const_opt_ossfuzz const_opt_ossfuzz.cpp ../fuzzer_common.cpp)
target_link_libraries(const_opt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

add_executable(strictasm_diff_ossfuzz strictasm_diff_ossfuzz.cpp yulFuzzerCommon.cpp)
target_link_libraries(strictasm_diff_ossfuzz PRIVATE libsolc evmasm yulInterpreter FuzzingEngine.a)