# Inherit default options
include("${CMAKE_CURRENT_LIST_DIR}/default.cmake")
# Enable Z3, disable CVC4
set(USE_Z3 ON CACHE BOOL "Enable Z3" FORCE)
set(USE_CVC4 OFF CACHE BOOL "Disable CVC4" FORCE)
# Build fuzzing binaries
set(OSSFUZZ ON CACHE BOOL "Enable fuzzer build" FORCE)
# Use libfuzzer as the fuzzing back-end
set(LIB_FUZZING_ENGINE "-fsanitize=fuzzer" CACHE STRING "Use libfuzzer back-end" FORCE)
# clang/libfuzzer specific flags for UBSan instrumentation
set(CMAKE_CXX_FLAGS "-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -I /usr/local/include/c++/v1 -fsanitize=undefined -fsanitize=fuzzer-no-link -stdlib=libc++" CACHE STRING "Custom compilation flags" FORCE)
# Link statically against boost libraries
set(BOOST_FOUND ON CACHE BOOL "" FORCE)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link against static Boost libraries" FORCE)
set(Boost_USE_STATIC_RUNTIME ON CACHE BOOL "Link against static Boost runtime library" FORCE)
