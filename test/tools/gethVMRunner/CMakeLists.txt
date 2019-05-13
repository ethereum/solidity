add_custom_target(gethVMRunner)
add_dependencies(gethVMRunner GethVMRunner)
file(GLOB gethVMRunner_SRC
        "*.cpp"
        )
add_library(GethVMRunner ${gethVMRunner_SRC})
target_link_libraries(GethVMRunner PRIVATE solidity langutil yul evmasm devcore)
