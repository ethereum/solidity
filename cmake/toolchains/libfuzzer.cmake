# Inherit default options
include("${CMAKE_CURRENT_LIST_DIR}/default.cmake")
# Enable Z3, disable CVC4
set(USE_Z3 ON CACHE BOOL "Enable Z3" FORCE)
set(USE_CVC4 OFF CACHE BOOL "Disable CVC4" FORCE)
# Build fuzzing binaries
set(OSSFUZZ ON CACHE BOOL "Enable fuzzer build" FORCE)
# Use libfuzzer as the fuzzing back-end
set(LIB_FUZZING_ENGINE "-fsanitize=fuzzer" CACHE STRING "Use libfuzzer back-end" FORCE)
# clang/libfuzzer specific flags for ASan instrumentation
set(CMAKE_CXX_FLAGS "-O1 -gline-tables-only -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link -stdlib=libstdc++" CACHE STRING "Custom compilation flags" FORCE)
