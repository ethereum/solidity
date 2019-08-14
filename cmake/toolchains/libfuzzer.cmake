# Inherit default options
include("${CMAKE_CURRENT_LIST_DIR}/default.cmake")
# Disable Z3 and CVC4 since none of the existing fuzzers need them
set(USE_Z3 OFF CACHE BOOL "" FORCE)
set(USE_CVC4 OFF CACHE BOOL "" FORCE)
# Build fuzzing binaries
set(OSSFUZZ 1)
# clang/libfuzzer specific flags for ASan instrumentation
set(CMAKE_CXX_FLAGS "-O1 -gline-tables-only -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link -stdlib=libstdc++")
