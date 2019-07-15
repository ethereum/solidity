add_custom_target(ossfuzz)
add_dependencies(ossfuzz
        solc_opt_ossfuzz
        solc_noopt_ossfuzz
        const_opt_ossfuzz
        strictasm_diff_ossfuzz
        strictasm_opt_ossfuzz
        strictasm_assembly_ossfuzz
        )

if (OSSFUZZ)
add_custom_target(ossfuzz_proto)
add_dependencies(ossfuzz_proto yul_proto_ossfuzz yul_proto_diff_ossfuzz)
endif()

if (OSSFUZZ)
    #[[FuzzingEngine.a is provided by oss-fuzz's Dockerized build environment]]
    add_executable(solc_opt_ossfuzz solc_opt_ossfuzz.cpp ../fuzzer_common.cpp)
    target_link_libraries(solc_opt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

    add_executable(solc_noopt_ossfuzz solc_noopt_ossfuzz.cpp ../fuzzer_common.cpp)
    target_link_libraries(solc_noopt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

    add_executable(const_opt_ossfuzz const_opt_ossfuzz.cpp ../fuzzer_common.cpp)
    target_link_libraries(const_opt_ossfuzz PRIVATE libsolc evmasm FuzzingEngine.a)

    add_executable(strictasm_diff_ossfuzz strictasm_diff_ossfuzz.cpp yulFuzzerCommon.cpp)
    target_link_libraries(strictasm_diff_ossfuzz PRIVATE libsolc evmasm yulInterpreter FuzzingEngine.a)

    add_executable(strictasm_opt_ossfuzz strictasm_opt_ossfuzz.cpp)
    target_link_libraries(strictasm_opt_ossfuzz PRIVATE yul FuzzingEngine.a)

    add_executable(strictasm_assembly_ossfuzz strictasm_assembly_ossfuzz.cpp)
    target_link_libraries(strictasm_assembly_ossfuzz PRIVATE yul FuzzingEngine.a)

    add_executable(yul_proto_ossfuzz yulProtoFuzzer.cpp protoToYul.cpp yulProto.pb.cc)
    target_include_directories(yul_proto_ossfuzz PRIVATE /usr/include/libprotobuf-mutator)
    target_link_libraries(yul_proto_ossfuzz PRIVATE yul
            protobuf-mutator-libfuzzer.a
            protobuf-mutator.a
            protobuf.a
            FuzzingEngine.a)

    add_executable(yul_proto_diff_ossfuzz yulProto_diff_ossfuzz.cpp yulFuzzerCommon.cpp protoToYul.cpp yulProto.pb.cc)
    target_include_directories(yul_proto_diff_ossfuzz PRIVATE /usr/include/libprotobuf-mutator)
    target_link_libraries(yul_proto_diff_ossfuzz PRIVATE yul
            yulInterpreter
            protobuf-mutator-libfuzzer.a
            protobuf-mutator.a
            protobuf.a
            FuzzingEngine.a)
else()
    add_library(solc_opt_ossfuzz
            solc_opt_ossfuzz.cpp
            ../fuzzer_common.cpp
            )
    target_link_libraries(solc_opt_ossfuzz PRIVATE libsolc evmasm)

    add_library(solc_noopt_ossfuzz
            solc_noopt_ossfuzz.cpp
            ../fuzzer_common.cpp
            )
    target_link_libraries(solc_noopt_ossfuzz PRIVATE libsolc evmasm)

    add_library(const_opt_ossfuzz
            const_opt_ossfuzz.cpp
            ../fuzzer_common.cpp)
    target_link_libraries(const_opt_ossfuzz PRIVATE libsolc evmasm)

    add_library(strictasm_diff_ossfuzz
            strictasm_diff_ossfuzz.cpp
            yulFuzzerCommon.cpp
            )
    target_link_libraries(strictasm_diff_ossfuzz PRIVATE libsolc evmasm yulInterpreter)

    add_library(strictasm_opt_ossfuzz
            strictasm_opt_ossfuzz.cpp
            )
    target_link_libraries(strictasm_opt_ossfuzz PRIVATE yul)

    add_library(strictasm_assembly_ossfuzz
            strictasm_assembly_ossfuzz.cpp
            )
    target_link_libraries(strictasm_assembly_ossfuzz PRIVATE yul)

#    add_executable(yul_proto_ossfuzz yulProtoFuzzer.cpp protoToYul.cpp yulProto.pb.cc)
#    target_include_directories(yul_proto_ossfuzz PRIVATE /src/libprotobuf-mutator /src/LPM/external.protobuf/include)
#    target_link_libraries(yul_proto_ossfuzz PRIVATE yul
#            protobuf-mutator-libfuzzer.a
#            protobuf-mutator.a
#            protobuf.a
#            FuzzingEngine.a)
#
#    add_executable(yul_proto_diff_ossfuzz yulProto_diff_ossfuzz.cpp yulFuzzerCommon.cpp protoToYul.cpp yulProto.pb.cc)
#    target_include_directories(yul_proto_diff_ossfuzz PRIVATE /src/libprotobuf-mutator /src/LPM/external.protobuf/include)
#    target_link_libraries(yul_proto_diff_ossfuzz PRIVATE yul
#            yulInterpreter
#            protobuf-mutator-libfuzzer.a
#            protobuf-mutator.a
#            protobuf.a
#            FuzzingEngine.a)
endif()
